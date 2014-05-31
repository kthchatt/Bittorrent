/*  GUI.c
 *	Author: insert name.
 *	
 *  Refactor: Robin Duda. ~RD
 *	
 *  	Merged duplicate functions and extended usability.
 *		Now able to use torrent data in tables.
 *  	Sorts torrents based on state at additions.
 *  	Identifies selected torrents with tab id and list id.
 *  	Updates displayed torrent data in separate thread.
 *  	Compressed column creating function.
 *  	Removed static addition of rows, added runtime addition and deletion. (used for full delete, and state moves)
 *  	Added more columns and made it easier to manage them.
 *  	Turned tab 'all' into tab 'inactive', to avoid redundancy and increase readability and simplicity.
 *  	Rows are moved between tabs on user interaction. (start/stop/remove)
 *		Fetching the RSS and MOTD from servers. Displayed in the GUI.
 *
 *  Todo: sorting on column click.
 *  Todo: adding torrent from path/patch checking for new files?
 *  Todo: add drag and drop? insert with path reference. (HOT)
 *  Todo: move rows between tabs on completion/queue-process.
 *  Todo: add queue handling.
 *  Todo: separate more code from the core GUI. (data loaders, anything not reliant on GTK)
 *  Todo: queue handling should work from the incomplete when there are slots in downloading. 
 */

//[todo: add tabs: peers, trackers, torrentinfo, logs.] ~RD

#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "netstat.h"
#include "swarm.h"
#include "MOTD.h"
#include "tracker.h"
#include "init_torrent.h"
#include "protocol_meta.h"
#include "urlparse.h"
#include "rss2.h"
#include "createfile.h"
#include "bitfield.h"
#include "ratiostat.h"
#include "math.h"

#define WINDOW_TITLE "ccTorrent v1.0b"

#define TORRENT_TABS 4
#define TAB_DOWNLOADING 1
#define TAB_SEEDING 2
#define TAB_COMPLETED 3
#define TAB_INACTIVE 4
#define true 1
#define false 0
#define FPS 5

 //following values is a part of the queue. ~RD
 #define MAX_DOWNLOADING 3
 #define MAX_SEEDING 20

 #define true 1
 #define false 0
 #define bool char

 enum
 {
 	STATE_DOWNLOADING,
 	STATE_SEEDING,
 	STATE_COMPLETED,
 	STATE_INACTIVE,
 	STATE_CREATING,
 };

enum {
	COL_ID = 0,
	COL_NAME = 1,
	COL_SIZE = 2,
	COL_DONE = 3,
	COL_STATUS = 4,
	COL_DOWNRATE = 5,
	COL_UPRATE = 6,
	COL_LEECHER = 7,
	COL_SEEDER = 8,
	COL_SWARM = 9,
	COL_RATIO = 10,
};

//torrent data to display, load from includes. ~RD
typedef struct
{
	int id;				//identifies a list item with this torrent.
	int state; 			//identifies the state of the torrent, 'downloading', 'seeding', 'completed', 'inactive', 'creating'
	int swarm_id;		//required for getting peer/seed/swarm size from swarm.c.
	char* filesize;
	char* status;
	torrent_info* tinfo;
} torrentlist_t;		//sort the list to implement priority.

//to add a column: increase the COUNT and add a NAME. ~RD
//in list_create add your data-type.
//in update_list to refresh the value.
//in torrentlist?
static int COLUMN_COUNT = 11;
static char* COLUMN_NAME[] = {"#", "Name", "Size", "Done", "Status", "Download", "Upload", "Leeches", "Seeds", "Swarm", "Ratio"};

//dynamic array of torrentlist.  ~RD
torrentlist_t* torrentlist; 
int torrentlist_count;
pthread_t update_thread, motd_thread, rss_thread;
pthread_mutex_t list_lock;

GtkWidget *tv_inactive, *tv_downloading, *tv_completed, *tv_seeding, *tv_rss;
GtkWidget *tlb_inactive, *tlb_downloading, *tlb_completed, *tlb_seeding;
GtkListStore *md_inactive, *md_downloading, *md_completed, *md_seeding, *md_rss;
GtkWidget *lb_netstat, *lb_motd;
GtkWidget *notebook;
rss_t rssfeed;

//calculate the filesize from bencodning.c .. ~RD
void torrent_size(torrent_info* tinfo, char* filesize)
{
	int i;
	long long int size = 0;

	memset(filesize, '\0', FORMATSTRING_LEN);

	for (i = 0; i < tinfo->_number_of_files; i++)
		size += tinfo->_file_length[i];

	url_filesize(filesize, size);		//format units, from: urlparse.c. ~RD
}

//add a row when the torrent-info already exists. ~RD
void row_add(int id, GtkListStore* ls)
{
	GtkTreeIter iter;
	lock(&list_lock);
   	gtk_list_store_append(ls, &iter);
   	gtk_list_store_set(ls, &iter,
   						COL_ID, id, 
   						COL_NAME, torrentlist[id].tinfo->_torrent_file_name, 
   						COL_SIZE, torrentlist[id].filesize, 
   						COL_DONE, "0.00%", 									//todo get this from fileman. 
   						COL_STATUS, torrentlist[id].status, 
   						COL_DOWNRATE, "N/A", 
   						COL_UPRATE, "N/A", 		
   						COL_LEECHER, 0, 
   						COL_SEEDER, 0, 
   						COL_SWARM, 0, 			
   						COL_RATIO, "", -1);
   	unlock(&list_lock);
}

//remove a row when the torrent-info already exists. ~RD
void row_delete(int id, GtkListStore* ls)
{
    GtkTreeIter  iter;
    gboolean     nextitem_exist;
    int item_id;

    lock(&list_lock);
	nextitem_exist = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(ls), &iter);

	//iterate the linked list until we reach the last item. 
    while (nextitem_exist)
 	{
     	gtk_tree_model_get(GTK_TREE_MODEL(ls), &iter, COL_ID, &item_id, -1);

     	if (id == item_id)
     	{
     		gtk_list_store_remove(ls, &iter);
     		break;
     	}

        nextitem_exist = gtk_tree_model_iter_next(GTK_TREE_MODEL(ls), &iter);
	}   
	unlock(&list_lock);
}

//add an info-item to list. ~RD
void list_add(char* status, torrent_info* tinfo, int state)
{
	int pos = torrentlist_count;
	if ((torrentlist = realloc(torrentlist, sizeof(torrentlist_t) * (pos + 1))) != NULL )
	{
		torrentlist[pos].status = malloc(60);
		torrentlist[pos].filesize = malloc(FORMATSTRING_LEN); 
		strcpy(torrentlist[pos].status, status);
		torrentlist[pos].swarm_id = -1;
		torrentlist[pos].tinfo = tinfo;
		torrentlist[pos].state = state;
		torrent_size(tinfo, torrentlist[pos].filesize);
		torrentlist_count++;

   		switch (state)
   		{
   			case STATE_COMPLETED: 	row_add(pos, md_completed); break;
   			case STATE_SEEDING: 	row_add(pos, md_seeding); break;
   			case STATE_DOWNLOADING: row_add(pos, md_downloading); break;
   			case STATE_INACTIVE:	row_add(pos, md_inactive); break;
   			case STATE_CREATING:    row_add(pos, md_inactive); break;
   		}
    }
}

//return selected tab as id. ~RD
int selected_tab(void)
{
	return (int) gtk_notebook_get_current_page((GtkNotebook*) notebook);
}

//returns the torrentlist-id of the selected torrent in the selected tab. ~RD
int selected_id(void)
{
	GtkTreeSelection* tsel;
	GtkTreeIter iter;
    GtkTreeModel* tm;
    GtkTreeModel* md;

    long int id;  
	int pgnum = selected_tab();

	switch (pgnum)
	{
		case TAB_INACTIVE: 		tsel 	= gtk_tree_view_get_selection((GtkTreeView*) tv_inactive); 
					 			md 		= (GtkTreeModel*) md_inactive; 
					  			 break;
		case TAB_DOWNLOADING: 	tsel 	= gtk_tree_view_get_selection((GtkTreeView*) tv_downloading); 
					  			md 		= (GtkTreeModel*) md_downloading; 
					  			 break;
		case TAB_COMPLETED:   	tsel 	= gtk_tree_view_get_selection((GtkTreeView*) tv_completed); 
					  			md 		= (GtkTreeModel*) md_completed; 
					  			 break;
		case TAB_SEEDING:     	tsel  	= gtk_tree_view_get_selection((GtkTreeView*) tv_seeding);
							   	md      = (GtkTreeModel*) md_seeding;
							     break;
	}

	if (0 < pgnum && pgnum <= TORRENT_TABS)
    	if (gtk_tree_selection_get_selected(tsel, &tm, &iter))
    	{
      		gtk_tree_model_get(md, &iter, COL_ID, &id, -1);
      		return (long int) id;
    	}
    return -1;
}

//get gtk-model from id. ~RD
GtkListStore* tab_model(int tab_id)
{
	switch (tab_id)
	{
		case TAB_DOWNLOADING: return md_downloading; 
		case TAB_SEEDING: return md_seeding;
		case TAB_COMPLETED: return md_completed;
		case TAB_INACTIVE: return md_inactive;
	}

	return NULL;
}


//update torrent items in liststore tab. This function (gtk_list_store_set) is EXTREMELY expensive. ~RD
void list_update(GtkListStore *ls)
{
    GtkTreeIter  iter;
    gboolean     nextitem_exist;
    int id;
    double percent = 0.0;
    char* progress = malloc(FORMATSTRING_LEN);
    char* ratio = malloc(FORMATSTRING_LEN);
    char netstat_down[FORMATSTRING_LEN];
	char netstat_up[FORMATSTRING_LEN];

	memset(ratio, '\0', FORMATSTRING_LEN);

	lock(&list_lock);
	nextitem_exist = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(ls), &iter);

	//iterate the linked list until the last item is reached.
    while (nextitem_exist)
 	{
		gtk_tree_model_get(GTK_TREE_MODEL(ls), &iter, COL_ID, &id, -1);	//get torrentlist id.

    	switch (torrentlist[id].state)
    	{
    		case STATE_CREATING: //get progress from createfile.c
    					percent = 100 * create_file_status(torrentlist[id].tinfo); 
    					if (fabs(percent - 100.0) <= DOUBLE_PRECISION) 
    					{
    						torrentlist[id].state = STATE_INACTIVE;
    						strcpy(torrentlist[id].status, "Ready");

    					}

    		break;		
    		case STATE_DOWNLOADING: //get progress based off the bitfield, calculated in bitfield.c
    				percent = bitfield_percent(swarm[torrentlist[id].swarm_id].bitfield, torrentlist[id].tinfo->_hash_length / 20); 
    				if (fabs(percent - (double) 100.0) <= DOUBLE_PRECISION)
    				{
    					unlock(&list_lock);
    					torrentlist[id].state = STATE_SEEDING;
    					row_delete(id, md_downloading);
    					strcpy(torrentlist[id].status, "Seeding");
						row_add(id, md_seeding);
						return; //we have modified the linked list, no more iteration! [verify: (lock(&list_lock))]
    				}
    				break;
    		case STATE_SEEDING: 
    				percent = (double) 100.0; break;	//[todo: get percent from ratio vs target ratio.]	
    	}

    	sprintf(progress, "%.2f%%", percent);
    	//[todo: implement logic from list_update_tabs here.]
    	
		netstat_formatbytes(torrentlist[id].tinfo->_info_hash, INPUT, netstat_down);
		netstat_formatbytes(torrentlist[id].tinfo->_info_hash, OUTPUT, netstat_up);
   		gtk_list_store_set(ls, &iter,
   						COL_DONE, progress,		//todo: get this from fileman 
   						COL_STATUS, torrentlist[id].status, 
   						COL_DOWNRATE, netstat_down, 
   						COL_UPRATE,   netstat_up, 		//get up/downrate from netstat.c 
   						COL_LEECHER, swarm_incomplete(torrentlist[id].swarm_id), 
   						COL_SEEDER, swarm_completed(torrentlist[id].swarm_id), 
   						COL_SWARM, swarm_peercount(torrentlist[id].swarm_id), 			//get these values from swarm.c 
   						COL_RATIO, netstat_formatratio(torrentlist[id].tinfo->_info_hash, ratio), -1);

       	nextitem_exist = gtk_tree_model_iter_next(GTK_TREE_MODEL(ls), &iter);
	}
    unlock(&list_lock);
	free(progress);
	free(ratio);
}


//timer for updating the MOTD after the retrieval timeout has passed. ~RD
void* motd_timer_thread(void* arg)
{
	char* response = (char*) malloc(MOTD_MAXLEN+1);

	MOTD_fetch(response);
	sleep(MOTD_TIMEOUT + 1);			//wait timeout.
	gtk_label_set_text((GtkLabel*) lb_motd, response);	
	return NULL;
}

//timer for updating the MOTD after the retrieval timeout has passed. ~RD
void* rss_timer_thread(void* arg)
{
	int i;
	GtkTreeIter iter;
	strcpy(rssfeed.host, "showrss.info"); 	//[todo: read host from configuration file.]
	strcpy(rssfeed.uri, "/feeds/27.rss");

	rss_fetch(&rssfeed);
	sleep(RSS_TIMEOUT + 1);			//wait for rss_fetch to timeout.
	
	for (i = 0; i < rssfeed.item_count && i < 9; i++){
	  	gtk_list_store_append(md_rss, &iter);
   		gtk_list_store_set(md_rss, &iter, 0, rssfeed.item[i].title, -1);
	}
	return NULL;
}

//update the torrents with data from modules at defined FPS. ~RD
void* gui_update_thread(void* arg)
{
	int pgnum;
	char* throughput_in = (char*) malloc(FORMATSTRING_LEN); 
	char* throughput_out = (char*) malloc(FORMATSTRING_LEN);
	char* throughput = (char*) malloc(75);

	while (true)
	{
		usleep((1000 / FPS) * 1000);

		pgnum = (int) gtk_notebook_get_current_page((GtkNotebook*) notebook);	//only update the active tab.
		switch (pgnum)
		{
			case TAB_INACTIVE: 		list_update(md_inactive);		break;
			case TAB_DOWNLOADING:	list_update(md_downloading);	break;
			case TAB_COMPLETED: 	list_update(md_completed); 		break;
			case TAB_SEEDING:	 	list_update(md_seeding); 		break;
		}

		netstat_throughput(INPUT, throughput_in);
		netstat_throughput(OUTPUT, throughput_out);
		sprintf(throughput, "D: %s, U: %s", throughput_in, throughput_out);
		gtk_label_set_text((GtkLabel*) lb_netstat, throughput);
	}

	free(throughput_in);
	free(throughput_out);
	free(throughput);
}


//create a list model by reference. ~RD
void list_create(GtkListStore **model)
{
	*model = gtk_list_store_new(COLUMN_COUNT, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, 
								G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_STRING); 
}

//handle doubleclick event on torrent. ~RD
void list_doubleclick(GtkTreeView *view, GtkTreePath *path, GtkTreeViewColumn *col, gpointer userdata)
{
	GtkTreeIter   iter;
	GtkTreeModel *model;
	char* folder = malloc(75);
	int id;

	model = gtk_tree_view_get_model(view);

	if (gtk_tree_model_get_iter(model, &iter, path))
	{
		gtk_tree_model_get(model, &iter, COL_ID, &id, -1);
		fflush(stdout);
		sprintf(folder, "xdg-open ./%s", torrentlist[id].tinfo->_file_name);
		system(folder);	//[todo: don't use system call, it's not secure.]
	}
	free(folder);
}


//if valid id is selected and valid tab (torrent tab) selected. ~RD
void torrent_start()
{
	int id, tab = selected_tab();
	if ((id = selected_id()) < 0)
		return;
	if (torrentlist[id].state == STATE_CREATING)
		return;
	//todo: show error message: "torrent is creating files"

	switch(tab)
	{
		case TAB_COMPLETED: row_delete(id, md_completed);
							strcpy(torrentlist[id].status, "Seeding");
							row_add(id, md_seeding); 
							netstat_track(torrentlist[id].tinfo->_info_hash, torrentlist[id].tinfo->_total_length);
							torrentlist[id].swarm_id = tracker_track(torrentlist[id].tinfo);
							torrentlist[id].state = STATE_SEEDING;
							 break;
		case TAB_INACTIVE:  
							if ((torrentlist[id].swarm_id = tracker_track(torrentlist[id].tinfo)) != -1)
							{
								row_delete(id, md_inactive);
								strcpy(torrentlist[id].status, "Downloading");
								row_add(id, md_downloading);
								netstat_track(torrentlist[id].tinfo->_info_hash, torrentlist[id].tinfo->_total_length);
								torrentlist[id].state = STATE_DOWNLOADING;	
							}
							else
								strcpy(torrentlist[id].status, "Queued"); //[todo: implement queue]
							
							 break;
	}

	fflush(stdout);
}

//if valid id is selected and valid tab (torrent tab) selected. ~RD
void torrent_stop()
{
	int id, tab = selected_tab();
	if ((id = selected_id()) < 0)
		return;

	switch (tab)
	{
		case TAB_SEEDING: 		row_delete(id, md_seeding); 
						  		row_add(id, md_completed); 
						  		netstat_untrack(torrentlist[id].tinfo->_info_hash);
						  		tracker_untrack(torrentlist[id].tinfo); 
						  		break;

		case TAB_DOWNLOADING: 	row_delete(id, md_downloading);
								row_add(id, md_inactive);
							  	netstat_untrack(torrentlist[id].tinfo->_info_hash);
							  	tracker_untrack(torrentlist[id].tinfo); 
							  	break;
	}
	
	fflush(stdout);
}

//if valid id is selected and valid tab (torrent tab) selected. ~RD
//remove object from all tabs.
void torrent_delete()
{
	int id, tab = selected_tab();
	if ((id = selected_id()) < 0)
		return;

	//todo add swarm_untrack and netstat_untrack.
	switch (tab)
	{
		case TAB_SEEDING: 		row_delete(id, md_seeding);     break;
		case TAB_DOWNLOADING: 	row_delete(id, md_downloading); break;
		case TAB_COMPLETED: 	row_delete(id, md_completed); break;
		case TAB_INACTIVE:	 	row_delete(id, md_inactive); break;
	}

	netstat_untrack(torrentlist[id].tinfo->_info_hash);

	free(torrentlist[id].status);
	free(torrentlist[id].filesize);
	//free self [todo:]

	fflush(stdout);
}

//keep id intact, reorder list. [todo: implement queue] ~RD
void torrent_prioritize()
{
	int id;
	if ((id = selected_id()) < 0)
		return;

	fflush(stdout);
}

//keep id intact, reorder list. [todo: implement queue] ~RD
void torrent_deprioritize()
{
	int id;
	if ((id = selected_id()) < 0)
		return;

	fflush(stdout);
}

void file_dialog(GtkWidget *junk, GtkTextBuffer *txtBuffer)
{
	char *filePath;
	GtkWidget *dialog, *win = NULL;
	GtkTextIter start, end;

	gtk_text_buffer_get_start_iter(txtBuffer, &start); 
	gtk_text_buffer_get_end_iter(txtBuffer, &end); 
	dialog = gtk_file_chooser_dialog_new ("Select folder",
					      GTK_WINDOW(win),
					      GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					      NULL);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT){
		// clear old text from textbox
		gtk_text_buffer_delete(txtBuffer, &start, &end);
		filePath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)); 
		// set text in textbox to selected folder path
		gtk_text_buffer_insert(txtBuffer, &start, filePath, strlen(filePath));
	}
		
	gtk_widget_destroy (dialog);
	gtk_widget_destroy(win);
}

void close_window(GtkWidget *junk, GtkWidget *window){
	gtk_widget_destroy(window);
}

void torrent_create()
{
	GtkWidget *window,
			  *table,
			  *fileLbl, *fileTxt, *fileBtn,
			  *trackerLbl, *trackerTxt,
			  *accept,
			  *cancel;
	GtkTextBuffer *txtBuffer;

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "New torrent"); 
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER); 
	gtk_container_border_width (GTK_CONTAINER (window), 25);
	gtk_widget_set_app_paintable(window, TRUE);

	table = gtk_table_new(3, 3, FALSE); 
	gtk_container_add(GTK_CONTAINER(window), table); 

	gtk_table_set_row_spacings(GTK_TABLE(table), 10);
	gtk_table_set_col_spacings(GTK_TABLE(table), 5);

	fileLbl = gtk_label_new("Directory path:");
	gtk_table_attach_defaults(GTK_TABLE(table), fileLbl, 0, 1, 0, 1);
	fileTxt = gtk_text_view_new();
	txtBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(fileTxt));
	gtk_table_attach_defaults(GTK_TABLE(table), fileTxt, 1, 2, 0, 1);
	fileBtn = gtk_button_new_with_label("...");
	gtk_table_attach_defaults(GTK_TABLE(table), fileBtn, 2, 3, 0, 1);
	g_signal_connect_object(G_OBJECT(fileBtn), "clicked", G_CALLBACK(file_dialog), G_OBJECT(txtBuffer), G_CONNECT_AFTER); 

	trackerLbl = gtk_label_new("Trackers:");
	gtk_table_attach_defaults(GTK_TABLE(table), trackerLbl, 0, 1, 1, 2);
	trackerTxt = gtk_text_view_new();
	gtk_table_attach_defaults(GTK_TABLE(table), trackerTxt, 1, 2, 1, 2);

	cancel = gtk_button_new_with_label("Cancel");
	gtk_table_attach_defaults(GTK_TABLE(table), cancel, 0, 1, 2, 3);

	accept = gtk_button_new_with_label("Create");
	gtk_table_attach_defaults(GTK_TABLE(table), accept, 1, 2, 2, 3);
	g_signal_connect_object(G_OBJECT(cancel), "clicked", G_CALLBACK(close_window), G_OBJECT(window), G_CONNECT_AFTER); 

	gtk_widget_set_size_request(fileTxt, 300, 1); // ????? same size as 28 ???
	gtk_widget_set_size_request(trackerTxt, 300, 70); // 

	gtk_widget_show_all(window);
}

void add_torrent(){
	GtkWidget *dialog, *win;
	torrent_info *torrent;
	char *filePath, *fileName, *fileSize, *tmp;

	torrent = malloc(sizeof(torrent_info));

/*-------------------------------- DEBUG ----------------------------------------*/
	/*fileName = malloc(50);
	strcpy(fileName, "tpb-afk.torrent");
	if (init_torrent(fileName, torrent) == 1)		//if decode_bencode returns with error, don't add to list. Display error dialog? ~RD
			{
				// convert filesize from long long int to char
				fileSize = malloc(sizeof(torrent->_total_length));
				memset(fileSize, '\0', sizeof(torrent->_total_length));
				sprintf(fileSize, "%lld", torrent->_total_length);
				// add torrent to list and initiate download
				list_add("Checking Files..", torrent, STATE_CREATING);
			}
			else
				free(torrent);

	free(fileName);

	return;*/
//------------------------------ END DEBUG ------------------------------------------

	win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	dialog = gtk_file_chooser_dialog_new ("Open File",
					      GTK_WINDOW(win),
					      GTK_FILE_CHOOSER_ACTION_OPEN,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					      NULL);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT){
		filePath = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog)); 
		if (strcmp(strrchr(filePath, '.'), ".torrent") == 0){ 	//Fixed:  (strrchr(filePath, '.')=="torent"): comparing pointer to "." in filePath with pointer to string "(t)orent". Fixed ~RD
			tmp = malloc(strlen(filePath));
			strcpy(tmp, filePath);
			tmp = strtok(tmp, "/");
			fileName = malloc(strlen(tmp));
			memset(fileName, '\0', strlen(tmp));
			while(tmp!=NULL){
				fileName = realloc(fileName, strlen(tmp));
				strcpy(fileName, tmp);
				tmp = strtok(NULL, "/");
			}
			tmp = realloc(tmp, strlen(filePath)+strlen(fileName)+6);
			sprintf(tmp, "cp %s %s", filePath, fileName); 	//use cp, not copy. ~RD
			//fprintf(stderr, "%s\n", tmp);
			system(tmp);									//don't use system here, unsecure. ~RD
			// get torrent info
			if (init_torrent(fileName, torrent) == 1)		//if decode_bencode returns with error, don't add to list. Display error dialog? ~RD
			{
				// convert filesize from long long int to char
				fileSize = malloc(sizeof(torrent->_total_length));
				memset(fileSize, '\0', sizeof(torrent->_total_length));
				sprintf(fileSize, "%lld", torrent->_total_length);
				// add torrent to list and initiate download
				list_add("Checking Files..", torrent, STATE_CREATING);
			}
			else
				free(torrent);
		}
	}
	gtk_widget_destroy(dialog); //some torrents will cause a segmentation fault here ~RD
}

//create MOTD display.
void MOTD(GtkWidget **label, GtkWidget **table) {
	*label = gtk_label_new("Loading MOTD ..."); // Label content
  	gtk_misc_set_alignment(GTK_MISC(*label), 0, 1); // Sets alignment of label
  	gtk_table_attach_defaults(GTK_TABLE(*table), *label, 0, 4, 10, 11); 
  	gtk_label_set_width_chars(GTK_LABEL(*label), 100);
}

//create total up/download - rates.
void netstat_label(GtkWidget **label, GtkWidget **table) {
	*label = gtk_label_new("Upload/Download speed");
  	gtk_misc_set_alignment(GTK_MISC (*label), 1, 1);
  	gtk_table_attach_defaults(GTK_TABLE (*table), *label, 4, 6, 10, 11);
}

//create a set of tabs.
void create_notebook (GtkWidget **table, GtkWidget **notebook) {
	*notebook = gtk_notebook_new(); // Creates new notebook
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(*notebook), GTK_POS_TOP); // Sets tab position
	gtk_table_attach_defaults(GTK_TABLE(*table), *notebook, 0,6,1,10); // Sets row and columns for notebook
}

//create a table to fill with widgets.
void create_table (GtkWidget **window, GtkWidget **table) {
	*table = gtk_table_new(11,6,TRUE); // Creates rows and columns for table
	gtk_container_add(GTK_CONTAINER(*window), *table); // Adds table to main window
	gtk_table_set_row_spacing(GTK_TABLE(*table),0,5); // Sets row space on row 5
}

//create a new renderer for cells.
void static enum_list(GtkWidget **tree_view, GtkListStore **model, GtkTreeViewColumn **column) {
	GtkCellRenderer   *renderer;

	*tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(*model));
	int i;

	for (i = 0; i < COLUMN_COUNT; i++)
	{
		renderer = gtk_cell_renderer_text_new();
		*column = gtk_tree_view_column_new_with_attributes(COLUMN_NAME[i], renderer, "text", i, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(*tree_view), *column);
	}
	g_object_unref(*model);
}

//create an RSS table.
void rss_table(GtkWidget *tbl){
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	md_rss = gtk_list_store_new(1, G_TYPE_STRING);
	tv_rss = gtk_tree_view_new_with_model(GTK_TREE_MODEL(md_rss));

	renderer = gtk_cell_renderer_text_new();
	column = gtk_tree_view_column_new_with_attributes("Feed", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tv_rss), column);

	gtk_table_attach_defaults(GTK_TABLE(tbl), tv_rss, 2, 3, 0, 5);

	rssfeed.host = malloc(MAX_URL_LEN);
	rssfeed.uri = malloc(MAX_URL_LEN);

	//[todo: implement double-click event.]
	//g_signal_connect(tv_rss, "row-activated", G_CALLBACK(rss_clicked), NULL);
}

//create the homescreen, with rss-feed and penguin.
void create_home (GtkWidget **label, GtkWidget **home_table, GtkWidget **view, GtkWidget **notebook) {
	*label = gtk_label_new("Home"); // Tab name
	*home_table = gtk_table_new(2,3,TRUE); //4?
	*view = gtk_label_new("RSS Table"); // Content of "Home" tab
	gtk_widget_set_usize(*view, 300, 30); // Max WIDTH x HEIGHT of content in tab
	gtk_misc_set_alignment(GTK_MISC(*view), 0, 0); // X & Y alignment of content
	gtk_misc_set_padding(GTK_MISC(*view), 10, 10); // Left/Right & Top/Bottom padding
	
	gtk_table_attach_defaults(GTK_TABLE(*home_table), *view, 2, 3, 0, 5);

	rss_table(*home_table);

	*view = gtk_label_new(""); // Content of "Home" tab
	gtk_widget_set_usize(*view, 300, 30); // Max WIDTH x HEIGHT of content in tab
	gtk_misc_set_alignment(GTK_MISC(*view), 0, 0); // X & Y alignment of conten
	gtk_misc_set_padding(GTK_MISC(*view), 10, 10); // Left/Right & Top/Bottom padding
	gtk_table_attach_defaults(GTK_TABLE(*home_table), *view, 0, 2, 0, 5);
	gtk_notebook_insert_page(GTK_NOTEBOOK(*notebook), *home_table, *label, 0); // Position of tab, in this case it's first

	GdkPixbuf* front_gpix;
 	GError** err = NULL;
 	GtkWidget* front_widget;
 	front_gpix = gdk_pixbuf_new_from_file("assets/front.png", err);
 	front_widget = gtk_image_new_from_pixbuf(front_gpix);
  	gtk_table_attach_defaults(GTK_TABLE(*home_table), front_widget, 1, 2, 0, 5);
}

//create a new torrent-tab with name and pos specified. ~RD
void create_torrent_tab(GtkWidget **label, GtkWidget **scrolled_window, GtkWidget **notebook, GtkWidget **treeView, char* name, int pos)
{
	*label = gtk_label_new(name);
	*scrolled_window = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(*scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); // X & Y scroll set to automatic
	gtk_container_add(GTK_CONTAINER(*scrolled_window), *treeView);
	gtk_notebook_insert_page (GTK_NOTEBOOK (*notebook), *scrolled_window, *label, pos); // Adds scrolled window to tab and positions it
}

//create the menu buttons and set up action listeners.
void create_menu (GtkWidget **toolbar, GtkWidget **table) {
	GtkToolItem	*play;
	GtkToolItem	*stop;
	GtkToolItem	*delete;
	GtkToolItem	*up;
	GtkToolItem	*down;
	GtkToolItem *create;
	GtkToolItem *add;

	*toolbar = gtk_toolbar_new(); // Creates new toolbar menu
  	gtk_toolbar_set_style(GTK_TOOLBAR(*toolbar), GTK_TOOLBAR_ICONS); 	// Sets style to display icons only
	gtk_table_attach_defaults (GTK_TABLE (*table), *toolbar, 0,2,0,1);  // Sets beginning position of toolbar

	//create the menu buttons.
	play = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PLAY); // Declares "PLAY" button
  	gtk_toolbar_insert(GTK_TOOLBAR(*toolbar), play, 0); // Adds "PLAY" button to the toolbar

	stop = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_STOP);
  	gtk_toolbar_insert(GTK_TOOLBAR(*toolbar), stop, 1);

	delete = gtk_tool_button_new_from_stock(GTK_STOCK_DELETE);
  	gtk_toolbar_insert(GTK_TOOLBAR(*toolbar), delete, 2);

	up = gtk_tool_button_new_from_stock(GTK_STOCK_GO_UP);
  	gtk_toolbar_insert(GTK_TOOLBAR(*toolbar), up, 3);

	down = gtk_tool_button_new_from_stock(GTK_STOCK_GO_DOWN);
  	gtk_toolbar_insert(GTK_TOOLBAR(*toolbar), down, 4);

  	create = gtk_tool_button_new_from_stock(GTK_STOCK_NEW);
	gtk_toolbar_insert(GTK_TOOLBAR(*toolbar), create, 5);

	add = gtk_tool_button_new_from_stock(GTK_STOCK_ADD);
	gtk_toolbar_insert(GTK_TOOLBAR(*toolbar), add, 6);

  	//add event handlers for the buttons.
	g_signal_connect(G_OBJECT(play), "clicked", G_CALLBACK(torrent_start), NULL);
	g_signal_connect(G_OBJECT(stop), "clicked", G_CALLBACK(torrent_stop), NULL);
	g_signal_connect(G_OBJECT(delete), "clicked", G_CALLBACK(torrent_delete), NULL);
	g_signal_connect(G_OBJECT(up), "clicked", G_CALLBACK(torrent_prioritize), NULL);
	g_signal_connect(G_OBJECT(down), "clicked", G_CALLBACK(torrent_deprioritize), NULL);
	g_signal_connect(G_OBJECT(create), "clicked", G_CALLBACK(torrent_create), NULL);
	g_signal_connect(G_OBJECT(add), "clicked", G_CALLBACK(add_torrent), NULL);

	g_signal_connect(tv_inactive, "row-activated", G_CALLBACK(list_doubleclick), NULL);
	g_signal_connect(tv_completed, "row-activated", G_CALLBACK(list_doubleclick), NULL);
	g_signal_connect(tv_downloading, "row-activated", G_CALLBACK(list_doubleclick), NULL);
	g_signal_connect(tv_seeding, "row-activated", G_CALLBACK(list_doubleclick), NULL);
}

//main. It's main.
int main (int argc, char *argv[])
{
	GtkWidget		*window;
	GtkWidget		*view;
	GtkWidget		*toolbar;
	GtkWidget		*table;
	GtkWidget		*home_table;
	GtkWidget		*label;
	GtkWidget		*scrolled_window;
	GtkTreeViewColumn 	*column;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL); 								//creates main window.
	gtk_window_set_title(GTK_WINDOW(window), WINDOW_TITLE); 				//title of main window.
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER); 			//main window is centered on start.
	gtk_window_set_icon_from_file(GTK_WINDOW(window), "assets/icon.png", NULL);
	gtk_container_border_width (GTK_CONTAINER (window), 10);					//inner border of window is set to 10.
	gtk_widget_set_app_paintable(window, TRUE);

	create_table(&window, &table);
	create_notebook(&table, &notebook);

// --------------List some torrents in TreeViews.----------------------------- ~RD
	torrentlist_count = 0;

	create_home(&label, &view, &home_table, &notebook);

	list_create(&md_inactive);
	list_create(&md_seeding);
	list_create(&md_completed);
	list_create(&md_downloading);

	enum_list(&tv_downloading, &md_downloading, &column);
	create_torrent_tab(&tlb_downloading, &scrolled_window, &notebook, &tv_downloading, "Downloading", 1);

	enum_list(&tv_seeding, &md_seeding, &column);
	create_torrent_tab(&tlb_seeding, &scrolled_window, &notebook, &tv_seeding, "Seeding", 2);	

	enum_list(&tv_completed, &md_completed, &column);
	create_torrent_tab(&tlb_completed, &scrolled_window, &notebook, &tv_completed, "Completed", 3);

	enum_list(&tv_inactive, &md_inactive, &column);
	create_torrent_tab(&tlb_inactive, &scrolled_window, &notebook, &tv_inactive, "Inactive", 4);

	//initializers
	netstat_initialize();
	swarm_initialize();

	//thread for updating the gui with new data.
	if (pthread_create(&update_thread, NULL, gui_update_thread, NULL))
			printf("\nUpdating your values in Thread. Failed.");

	//thread for fetching the motd.
	if (pthread_create(&motd_thread, NULL, motd_timer_thread, NULL))
			printf("\nWaiting for MOTD in Thread. Failed.");

	//thread for retrieving the rss-feed.
	if (pthread_create(&rss_thread, NULL, rss_timer_thread, tv_rss))
			printf("\nWaiting for Feed in Thread. Failed.");

//---------------------------------------------------------------------------  ~RD

	MOTD(&lb_motd, &table);
	netstat_label(&lb_netstat, &table);
	create_menu(&toolbar, &table);

	gtk_widget_show_all(window);
	g_signal_connect_swapped(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);


	gtk_main();
	return 0;
}