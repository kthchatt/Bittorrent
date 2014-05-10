#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>

/*  GUI2.c
 *	Author: insert name.
 *	
 *  Refactor: Robin Duda.
 *	
 *  Merged duplicate functions and extended usability.
 *	Now able to use torrent data in tables.
 *  Sorts torrents based on state.
 *  Identifies selected torrents with tab id and list id.
 *  Updates torrent statuses.
 */

//add tab: seeding, peers, trackers, torrentinfo, logs?
#define TAB_ALL 1
#define TAB_DOWNLOADING 2
#define TAB_COMPLETED 3

#define true 1
#define false 0
#define FPS 20

//the way GTK mixes a hundred snakes with a few camels, it is impious. ~RD
//#define widget_t GtkWidget ?

enum {
	COL_ID = 0,
	COL_NAME = 1,
	COL_SIZE = 2,
	COL_DONE = 3,
	COL_STATUS = 4
};

//torrent data to display, load from includes. ~RD
typedef struct
{
	int id;
	char* size;
	char* done;
	char* status;
	char* name;
	char* info_hash;
} torrentlist_t;

//todo: add more tabs? log, peers, trackers? ~RD

//dynamic array of torrentlist.  ~RD
torrentlist_t* torrentlist; 
int torrentlist_count;

pthread_t update_thread;

//global required, multiple pointers to retain references. ~RD
GtkWidget *tv_all, *tv_downloading, *tv_completed;
GtkWidget *tlb_all, *tlb_downloading, *tlb_completed;
GtkListStore *md_all, *md_downloading, *md_completed;
GtkWidget *notebook;
double pc = 0.00;

//update torrent item in liststore tab. This functions is EXTREMELY expensive. ~RD
void update_list(GtkListStore *liststore)
  {
    GtkTreeIter  iter;
    gboolean     nextitem_exist;
    char percent[20];

    sprintf(percent, "%.2f %%", pc);
    g_return_if_fail(liststore != NULL);
    nextitem_exist = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(liststore), &iter);

    while (nextitem_exist)
    {
    	//todo: fetch actual values, percent, peercount, swarm-size, leeches, seeders.
    	//get id in column here, from use torrent-info to get data from modules.

       gtk_list_store_set(liststore, &iter, 3, percent, -1);
       nextitem_exist = gtk_tree_model_iter_next(GTK_TREE_MODEL(liststore), &iter);
    }
  }


//update the torrents with data from modules at defined FPS. ~RD
void* gui_update_thread(void* arg)
{
	int pgnum;

	while (true)
	{
		usleep((1000 / FPS) * 1000);
		pc += 0.04;

		//todo: fetch actual values.
		//optimization: only update the active tab.
		pgnum = (int) gtk_notebook_get_current_page((GtkNotebook*) notebook);
		switch (pgnum)
		{
			case TAB_ALL: 			update_list(md_all);			break;
			case TAB_DOWNLOADING:	update_list(md_downloading);	break;
			case TAB_COMPLETED: 	update_list(md_completed); 		break;
		}
	}
}

//compile a list of info-items based on status, * = any.  ~RD
void list_compile(GtkListStore **model, char* status)
{
	int i;
	*model = gtk_list_store_new(5, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	for(i = 0; i < torrentlist_count; i++) 
	{
		if (strcmp(torrentlist[i].status, status) == 0 || strcmp(status, "*") == 0)
		{
			printf("\nAdding Torrent: #%d, %s\t%s\t\t%s", torrentlist[i].id, torrentlist[i].name, torrentlist[i].size, torrentlist[i].done);
			gtk_list_store_insert_with_values(*model, NULL, -1, COL_ID, torrentlist[i].id, COL_NAME, torrentlist[i].name, COL_SIZE, torrentlist[i].size, 
										 	  COL_DONE, torrentlist[i].done, COL_STATUS, torrentlist[i].status, -1);
		}
	}
}

//add an info-item to list. ~RD
void list_add(char* name, char* status, char* size, char* done, char* info_hash)
{
	GtkTreeModel *model;
	GtkTreeIter iter;

	if ((torrentlist = realloc(torrentlist, sizeof(torrentlist_t) * (torrentlist_count + 1))) != NULL )
	{
		torrentlist[torrentlist_count].size = malloc(8);
		torrentlist[torrentlist_count].done = malloc(8);
		torrentlist[torrentlist_count].status = malloc(16);
		torrentlist[torrentlist_count].name = malloc(32);
		torrentlist[torrentlist_count].info_hash = malloc(21);

		strcpy(torrentlist[torrentlist_count].info_hash, info_hash);
		strcpy(torrentlist[torrentlist_count].size, size);
		strcpy(torrentlist[torrentlist_count].done, done);
		strcpy(torrentlist[torrentlist_count].status, status);
		strcpy(torrentlist[torrentlist_count].name, name);
		torrentlist[torrentlist_count].id = torrentlist_count;

		torrentlist_count++;
	}

	//todo: specify md_ target for insertion. md_all && (md_completed || md_seeding || md_downloading) ~RD
   	gtk_list_store_append(md_all, &iter);
   	gtk_list_store_set(md_all, &iter, COL_ID, torrentlist_count-1, COL_NAME, name, COL_SIZE, size, 
   										COL_DONE, done, COL_STATUS, status, -1);
}

void MOTD(GtkWidget **label, GtkWidget **table) {
	*label = gtk_label_new("MOTD goes here."); // Label content
  	gtk_misc_set_alignment(GTK_MISC (*label), 0, 1); // Sets alignment of label
  	gtk_table_attach_defaults (GTK_TABLE (*table), *label, 0, 1, 10, 11); // Sets beginning position of label in table
}

void netstat_label(GtkWidget **label, GtkWidget **table) {
	*label = gtk_label_new("Upload/Download speed");
  	gtk_misc_set_alignment(GTK_MISC (*label), 1, 1);
  	gtk_table_attach_defaults(GTK_TABLE (*table), *label, 5, 6, 10, 11);
}

void create_notebook (GtkWidget **table, GtkWidget **notebook) {
	*notebook = gtk_notebook_new(); // Creates new notebook
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK(*notebook), GTK_POS_TOP); // Sets tab position
	gtk_table_attach_defaults(GTK_TABLE(*table), *notebook, 0,6,1,10); // Sets row and columns for notebook
}

void create_table (GtkWidget **window, GtkWidget **table) {
	*table = gtk_table_new(11,6,TRUE); // Creates rows and columns for table
	gtk_container_add(GTK_CONTAINER(*window), *table); // Adds table to main window
	gtk_table_set_row_spacing(GTK_TABLE(*table),0,5); // Sets row space on row 5
}

void static enum_list(GtkWidget **tree_view, GtkListStore **model, GtkTreeViewColumn **column) {
	GtkCellRenderer   *renderer;

	*tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(*model));

	renderer = gtk_cell_renderer_text_new();
	*column = gtk_tree_view_column_new_with_attributes("#", renderer, "text", COL_ID, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(*tree_view), *column);

	renderer = gtk_cell_renderer_text_new();
	*column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", COL_NAME, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(*tree_view), *column);

	renderer = gtk_cell_renderer_text_new();
	*column = gtk_tree_view_column_new_with_attributes("Size", renderer, "text", COL_SIZE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(*tree_view), *column);

	renderer = gtk_cell_renderer_text_new();
	*column = gtk_tree_view_column_new_with_attributes("Done", renderer, "text", COL_DONE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(*tree_view), *column);

	renderer = gtk_cell_renderer_text_new();
	*column = gtk_tree_view_column_new_with_attributes("Status", renderer, "text", COL_STATUS, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(*tree_view), *column);

	g_object_unref(*model);
}

void create_home (GtkWidget **label, GtkWidget **home_table, GtkWidget **view, GtkWidget **notebook) {
	*label = gtk_label_new("Home"); // Tab name
	*home_table = gtk_table_new(1,3,TRUE);
	*view = gtk_label_new("RSS Table"); // Content of "Home" tab
	gtk_widget_set_usize(*view, 300, 30); // Max WIDTH x HEIGHT of content in tab
	gtk_misc_set_alignment(GTK_MISC(*view), 0, 0); // X & Y alignment of content
	gtk_misc_set_padding(GTK_MISC(*view), 10, 10); // Left/Right & Top/Bottom padding
	gtk_table_attach_defaults(GTK_TABLE(*home_table), *view, 2, 3, 0, 1);
	*view = gtk_label_new("Counters Table"); // Content of "Home" tab
	gtk_widget_set_usize(*view, 300, 30); // Max WIDTH x HEIGHT of content in tab
	gtk_misc_set_alignment(GTK_MISC(*view), 0, 0); // X & Y alignment of content
	gtk_misc_set_padding(GTK_MISC(*view), 10, 10); // Left/Right & Top/Bottom padding
	gtk_table_attach_defaults(GTK_TABLE(*home_table), *view, 0, 2, 0, 1);
	gtk_notebook_insert_page(GTK_NOTEBOOK(*notebook), *home_table, *label, 0); // Position of tab, in this case it's first
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

//returns the torrentlist-id of the selected torrent in the selected tab. ~RD
int selected_id(void)
{
	GtkTreeSelection* tsel;
	GtkTreeIter iter;
    GtkTreeModel* tm;
    GtkTreeModel* md;
    long int* id = malloc(8);
	int pgnum = (int) gtk_notebook_get_current_page((GtkNotebook*) notebook);

	switch (pgnum)
	{
		case TAB_ALL: tsel 			= gtk_tree_view_get_selection((GtkTreeView*) tv_all); 
					  md 			= (GtkTreeModel*) md_all; 
					  break;
		case TAB_DOWNLOADING: tsel 	= gtk_tree_view_get_selection((GtkTreeView*) tv_downloading); 
					  md 			= (GtkTreeModel*) md_downloading; 
					  break;
		case TAB_COMPLETED: tsel 	= gtk_tree_view_get_selection((GtkTreeView*) tv_completed); 
					  md 			= (GtkTreeModel*) md_completed; 
					  break;
	}

	if (0 < pgnum && pgnum < 4)
    	if (gtk_tree_selection_get_selected(tsel, &tm, &iter))
    	{
      		gtk_tree_model_get(md, &iter, COL_ID, &id, -1);
      		return (long int) id;
    	}
    return -1;
}

//if valid id is selected and valid tab (torrent tab) selected. ~RD
void torrent_start()
{
	int id;
	if ((id = selected_id()) < 0)
		return;

	printf("\nTracking torrent %s", torrentlist[id].info_hash);
	strcpy(torrentlist[id].name, "UpdatedName");
	fflush(stdout);
}

//if valid id is selected and valid tab (torrent tab) selected. ~RD
void torrent_stop()
{
	int id;
	if ((id = selected_id()) < 0)
		return;
	
	printf("\nStopping torrent %s", torrentlist[id].info_hash);
}

//if valid id is selected and valid tab (torrent tab) selected. ~RD
void torrent_delete()
{
	int id;
	if ((id = selected_id()) < 0)
		return;

	printf("\nDeleting torrent %s", torrentlist[id].info_hash);
}

//keep id intact, reorder list. ~RD
void torrent_prioritize()
{
	int id;
	if ((id = selected_id()) < 0)
		return;
	g_print ("You clicked on the arrow up button!\n");
}

//keep id intact, reorder list. ~RD
void torrent_deprioritize()
{
	int id;
	if ((id = selected_id()) < 0)
		return;
	g_print ("You clicked on the arrow down button!\n");
}

void create_menu (GtkWidget **toolbar, GtkWidget **table) {
	GtkToolItem	*play;
	GtkToolItem	*stop;
	GtkToolItem	*delete;
	GtkToolItem	*up;
	GtkToolItem	*down;

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

  	//add event handlers for the buttons.
	g_signal_connect(G_OBJECT(play), "clicked", G_CALLBACK(torrent_start), NULL);
	g_signal_connect(G_OBJECT(stop), "clicked", G_CALLBACK(torrent_stop), NULL);
	g_signal_connect(G_OBJECT(delete), "clicked", G_CALLBACK(torrent_delete), NULL);
	g_signal_connect(G_OBJECT(up), "clicked", G_CALLBACK(torrent_prioritize), NULL);
	g_signal_connect(G_OBJECT(down), "clicked", G_CALLBACK(torrent_deprioritize), NULL);
}

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

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL); // Creates main window
	gtk_window_set_title(GTK_WINDOW(window), "Torrent"); // Title of main window
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER); // Main window is centered on start
	gtk_container_border_width (GTK_CONTAINER (window), 10);// Inner border of window is set to 10
	gtk_widget_set_app_paintable(window, TRUE);

	create_table(&window, &table);
	create_notebook(&table, &notebook);

// --------------List some torrents in TreeViews.----------------------------- ~RD
	torrentlist_count = 0;

	//add info-item to list, use fileman to populate at startup. ~RD
	//name, status, size, completed and hash should be found in fileman-torrentloader.
	/*list_add("Photoflop CS7", 			"Completed", 	"8.43 GB", 	 "100.00%", "----  INFOHASH  ----");
	list_add("World of Catcraft", 		"Downloading", 	"12.47 GB",  "68.13%",  "----  INFOHASH  ----");
	list_add("The.Shrimpsons S08E03", 	"Downloading", 	"413.89 MB", "12.04%",  "----  INFOHASH  ----");
	list_add("EBook_ASM_Cookbook", 		"Downloading", 	"55.10 MB",  "97.89%",  "----  INFOHASH  ----");
	list_add("The.Shrimpsons S08E04", 	"Completed", 	"374.95 MB", "100.00%", "----  INFOHASH  ----");
	list_add("The.Shrimpsons S08E05", 	"Completed", 	"415.10 MB", "100.00%", "----  INFOHASH  ----");*/
	
	//int k;
	//for (k = 0; k < 100; k++)
	//	list_add("Super-Advanced-IDE", 		"Downloading", 	"3.10 GB",    "97.89%", "----  INFOHASH  ----");

	create_home(&label, &view, &home_table, &notebook);

	list_compile(&md_all, "*");
	enum_list(&tv_all, &md_all, &column);
	create_torrent_tab(&tlb_all, &scrolled_window, &notebook, &tv_all, "All", 1);

	list_compile(&md_downloading, "Downloading");
	enum_list(&tv_downloading, &md_downloading, &column);
	create_torrent_tab(&tlb_downloading, &scrolled_window, &notebook, &tv_downloading, "Downloading", 2);	

	list_compile(&md_completed, "Completed");
	enum_list(&tv_completed, &md_completed, &column);
	create_torrent_tab(&tlb_completed, &scrolled_window, &notebook, &tv_completed, "Completed", 3);

	list_add("Photoflop CS7", 			"Completed", 	"8.43 GB", 	 "100.00%", "----  INFOHASH  ----");
	list_add("World of Catcraft", 		"Downloading", 	"12.47 GB",  "68.13%",  "----  INFOHASH  ----");
	list_add("The.Shrimpsons S08E03", 	"Downloading", 	"413.89 MB", "12.04%",  "----  INFOHASH  ----");
	list_add("EBook_ASM_Cookbook", 		"Downloading", 	"55.10 MB",  "97.89%",  "----  INFOHASH  ----");
	list_add("The.Shrimpsons S08E04", 	"Completed", 	"374.95 MB", "100.00%", "----  INFOHASH  ----");
	list_add("The.Shrimpsons S08E05", 	"Completed", 	"415.10 MB", "100.00%", "----  INFOHASH  ----");
//---------------------------------------------------------------------------  ~RD

	MOTD(&label, &table);
	netstat_label(&label, &table);
	create_menu(&toolbar, &table);

// Show window widget and it's child widgets
	gtk_widget_show_all(window);
	g_signal_connect_swapped(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

	if (!(pthread_create(&update_thread, NULL, gui_update_thread, NULL)))
			printf("\nTracking your network statistics.");

	gtk_main();

	return 0;
}