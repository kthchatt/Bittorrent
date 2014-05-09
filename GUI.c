#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>

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
} torrentlist_t;

//todo: add more tabs? log, peers, trackers? ~RD

//dynamic array of torrentlist.  ~RD
torrentlist_t* torrentlist; 
int torrentlist_count;


//compile a list of info-items based on status, * = any.  ~RD
void list_compile(GtkListStore **model, char* status)
{
	int i;
	*model = gtk_list_store_new(5, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	for(i = 0; i < torrentlist_count; i++) 
	{
		if (strcmp(torrentlist[i].status, status) == 0 || strcmp(status, "*") == 0)
		{
			printf("\nTorrent: #%d, %s\t%s\t\t%s", torrentlist[i].id, torrentlist[i].name, torrentlist[i].size, torrentlist[i].done);
			gtk_list_store_insert_with_values(*model, NULL, -1, COL_ID, torrentlist[i].id, COL_NAME, torrentlist[i].name, COL_SIZE, torrentlist[i].size, 
										 	  COL_DONE, torrentlist[i].done, COL_STATUS, torrentlist[i].status, -1);
		}
	}
}

//add an info-item to list. ~RD
void list_add(char* name, char* status, char* size, char* done)
{
	printf("\nSize: %lu", sizeof(torrentlist_t) * (torrentlist_count + 1)); fflush(stdout);

	if ((torrentlist = realloc(torrentlist, sizeof(torrentlist_t) * (torrentlist_count + 1))) != NULL )
	{
		torrentlist[torrentlist_count].size = malloc(8);
		torrentlist[torrentlist_count].done = malloc(8);
		torrentlist[torrentlist_count].status = malloc(16);
		torrentlist[torrentlist_count].name = malloc(32);

		strcpy(torrentlist[torrentlist_count].size, size);
		strcpy(torrentlist[torrentlist_count].done, done);
		strcpy(torrentlist[torrentlist_count].status, status);
		strcpy(torrentlist[torrentlist_count].name, name);
		torrentlist[torrentlist_count].id = torrentlist_count;

		torrentlist_count++;
	}
}

void MOTD(GtkWidget **label, GtkWidget **table) {
	*label = gtk_label_new ("MOTD goes here."); // Label content
  	gtk_misc_set_alignment (GTK_MISC (*label), 0, 1); // Sets alignment of label
  	gtk_table_attach_defaults (GTK_TABLE (*table), *label, 0, 1, 10, 11); // Sets beginning position of label in table
}

void duStats(GtkWidget **label, GtkWidget **table) {
	*label = gtk_label_new ("Upload/Download speed");
  	gtk_misc_set_alignment (GTK_MISC (*label), 1, 1);
  	gtk_table_attach_defaults (GTK_TABLE (*table), *label, 5, 6, 10, 11);
}

void createNotebook (GtkWidget **table, GtkWidget **notebook) {
	*notebook = gtk_notebook_new (); // Creates new notebook
	gtk_notebook_set_tab_pos (GTK_NOTEBOOK (*notebook), GTK_POS_TOP); // Sets tab position
	gtk_table_attach_defaults(GTK_TABLE(*table), *notebook, 0,6,1,10); // Sets row and columns for notebook
}

void createTable (GtkWidget **window, GtkWidget **table) {
	*table = gtk_table_new(11,6,TRUE); // Creates rows and columns for table
	gtk_container_add (GTK_CONTAINER (*window), *table); // Adds table to main window
	gtk_table_set_row_spacing(GTK_TABLE (*table),0,5); // Sets row space on row 5
}

void static createList (GtkListStore **model) {
	int i = 0;
	static int torrentID = 1;

	*model = gtk_list_store_new(5, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	for(i=0;i<20;i++) {
		gtk_list_store_insert_with_values(*model, NULL, -1, COL_ID, torrentID, COL_NAME, "Active", COL_SIZE, "20 MB", COL_DONE, "67%", COL_STATUS, "Downloading", -1);
		torrentID++;

		gtk_list_store_insert_with_values(*model, NULL, -1, COL_ID, torrentID, COL_NAME, "Completed", COL_SIZE, "50 MB", COL_DONE, "100%", COL_STATUS, "Completed", -1);
		torrentID++;
	}
}

void static showList (GtkWidget **treeView, GtkListStore **model, GtkTreeViewColumn **column) {
	GtkCellRenderer   *renderer;

	*treeView = gtk_tree_view_new_with_model(GTK_TREE_MODEL(*model));

	renderer = gtk_cell_renderer_text_new();
	*column = gtk_tree_view_column_new_with_attributes("#", renderer, "text", COL_ID, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(*treeView), *column);

	renderer = gtk_cell_renderer_text_new();
	*column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", COL_NAME, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(*treeView), *column);

	renderer = gtk_cell_renderer_text_new();
	*column = gtk_tree_view_column_new_with_attributes("Size", renderer, "text", COL_SIZE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(*treeView), *column);

	renderer = gtk_cell_renderer_text_new();
	*column = gtk_tree_view_column_new_with_attributes("Done", renderer, "text", COL_DONE, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(*treeView), *column);

	renderer = gtk_cell_renderer_text_new();
	*column = gtk_tree_view_column_new_with_attributes("Status", renderer, "text", COL_STATUS, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(*treeView), *column);

	g_object_unref(*model);
}

void createHome (GtkWidget **label, GtkWidget **home_table, GtkWidget **view, GtkWidget **notebook) {
	*label = gtk_label_new ("Home"); // Tab name
	*home_table = gtk_table_new(1,3,TRUE);
	*view = gtk_label_new ("RSS Table"); // Content of "Home" tab
	gtk_widget_set_usize(*view, 300, 30); // Max WIDTH x HEIGHT of content in tab
	gtk_misc_set_alignment (GTK_MISC (*view), 0, 0); // X & Y alignment of content
	gtk_misc_set_padding (GTK_MISC (*view), 10, 10); // Left/Right & Top/Bottom padding
	gtk_table_attach_defaults (GTK_TABLE (*home_table), *view, 2, 3, 0, 1);
	*view = gtk_label_new ("Counters Table"); // Content of "Home" tab
	gtk_widget_set_usize(*view, 300, 30); // Max WIDTH x HEIGHT of content in tab
	gtk_misc_set_alignment (GTK_MISC (*view), 0, 0); // X & Y alignment of content
	gtk_misc_set_padding (GTK_MISC (*view), 10, 10); // Left/Right & Top/Bottom padding
	gtk_table_attach_defaults (GTK_TABLE (*home_table), *view, 0, 2, 0, 1);
	gtk_notebook_insert_page (GTK_NOTEBOOK (*notebook), *home_table, *label, 0); // Position of tab, in this case it's first
}

void createAll (GtkWidget **label, GtkWidget **scrolled_window, GtkWidget **notebook, GtkWidget **treeView) {
	*label = gtk_label_new ("All");
	*scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (*scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC); // X & Y scroll set to automatic
	gtk_container_add(GTK_CONTAINER(*scrolled_window), *treeView);
	gtk_notebook_insert_page (GTK_NOTEBOOK (*notebook), *scrolled_window, *label, 1); // Adds scrolled window to tab and positions it
}

void createActive (GtkWidget **label, GtkWidget **scrolled_window, GtkWidget **notebook, GtkWidget **treeView) {
	*label = gtk_label_new ("Active");
	*scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (*scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(*scrolled_window), *treeView);
	gtk_notebook_insert_page (GTK_NOTEBOOK (*notebook), *scrolled_window, *label, 2);
}

void createCompleted (GtkWidget **label, GtkWidget **scrolled_window, GtkWidget **notebook, GtkWidget **treeView) {
	*label = gtk_label_new ("Completed");
	*scrolled_window = gtk_scrolled_window_new (NULL, NULL);
	gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (*scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_container_add(GTK_CONTAINER(*scrolled_window), *treeView);
	gtk_notebook_insert_page (GTK_NOTEBOOK (*notebook), *scrolled_window, *label, 3);
}

void playAction()
{
	g_print ("You clicked on the play button!\n");
}

void stopAction()
{
	g_print ("You clicked on the stop button!\n");
}

void deleteAction()
{
	g_print ("You clicked on the delete button!\n");
}

void arrowUpAction()
{
	g_print ("You clicked on the arrow up button!\n");
}

void arrowDownAction()
{
	g_print ("You clicked on the arrow down button!\n");
}

void createMenu (GtkWidget **toolbar, GtkWidget **table) {
	GtkToolItem	*play;
	GtkToolItem	*stop;
	GtkToolItem	*delete;
	GtkToolItem	*up;
	GtkToolItem	*down;

	*toolbar = gtk_toolbar_new(); // Creates new toolbar menu
  	gtk_toolbar_set_style(GTK_TOOLBAR(*toolbar), GTK_TOOLBAR_ICONS); // Sets style to display icons only
	gtk_table_attach_defaults (GTK_TABLE (*table), *toolbar, 0,2,0,1); // Sets beginning position of toolbar

	// Play button
	play = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_PLAY); // Declares "PLAY" button
  	gtk_toolbar_insert(GTK_TOOLBAR(*toolbar), play, 0); // Adds "PLAY" button to the toolbar

	// Stop button
	stop = gtk_tool_button_new_from_stock(GTK_STOCK_MEDIA_STOP);
  	gtk_toolbar_insert(GTK_TOOLBAR(*toolbar), stop, 1);

	// Delete button
	delete = gtk_tool_button_new_from_stock(GTK_STOCK_DELETE);
  	gtk_toolbar_insert(GTK_TOOLBAR(*toolbar), delete, 2);

	// Arrow up
	up = gtk_tool_button_new_from_stock(GTK_STOCK_GO_UP);
  	gtk_toolbar_insert(GTK_TOOLBAR(*toolbar), up, 3);

	// Arrow down
	down = gtk_tool_button_new_from_stock(GTK_STOCK_GO_DOWN);
  	gtk_toolbar_insert(GTK_TOOLBAR(*toolbar), down, 4);

	g_signal_connect(G_OBJECT(play), "clicked", G_CALLBACK(playAction), NULL);
	g_signal_connect(G_OBJECT(stop), "clicked", G_CALLBACK(stopAction), NULL);
	g_signal_connect(G_OBJECT(delete), "clicked", G_CALLBACK(deleteAction), NULL);
	g_signal_connect(G_OBJECT(up), "clicked", G_CALLBACK(arrowUpAction), NULL);
	g_signal_connect(G_OBJECT(down), "clicked", G_CALLBACK(arrowDownAction), NULL);
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
	GtkWidget		*notebook;
	GtkWidget		*treeView;
	GtkListStore		*model;
	GtkTreeViewColumn 	*column;

// Initiation of GTK+ library
	gtk_init (&argc, &argv);

// Main window
	window = gtk_window_new (GTK_WINDOW_TOPLEVEL); // Creates main window
	gtk_window_set_title(GTK_WINDOW(window), "Torrent"); // Title of main window
	//gtk_window_set_default_size(GTK_WINDOW(window), 1000, 600);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER); // Main window is centered on start
	gtk_container_border_width (GTK_CONTAINER (window), 10);// Inner border of window is set to 10
	gtk_widget_set_app_paintable(window, TRUE);

// Create table
	createTable (&window, &table);

// Create Notebook
	createNotebook (&table, &notebook);


// List some torrents in TreeViews. --------------------------------------
	torrentlist_count = 0;

	//add info-item to list.
	list_add("Photoflop CS7", "Completed", "8.43 GB", "100.00%");
	list_add("World of Catcraft", "Downloading", "12.47 GB", "68.13%");
	list_add("The.Shrimpsons S08E03", "Downloading", "413.89 MB", "12.04%");
	list_add("EBook_ASM_Cookbook", "Downloading", "55.10 MB", "97.89%");
	list_add("The.Shrimpsons S08E04", "Completed", "374.95 MB", "100.00%");
	list_add("The.Shrimpsons S08E05", "Completed", "415.10 MB", "100.00%");
	list_add("Super-Advanced-IDE", "Downloading", "3.10 GB", "97.89%");

	//create Tabs.
	createList(&model);
	showList(&treeView, &model, &column);
	createHome (&label, &view, &home_table, &notebook);

	list_compile(&model, "*");
	showList(&treeView, &model, &column);
	createAll(&label, &scrolled_window, &notebook, &treeView);

	list_compile(&model, "Downloading");
	showList(&treeView, &model, &column);
	createActive(&label, &scrolled_window, &notebook, &treeView);	

	list_compile(&model, "Completed");
	showList(&treeView, &model, &column);
	createCompleted(&label, &scrolled_window, &notebook, &treeView);
//---------------------------------------------------------------------------



// Tabs
	/*createList(&model);
	showList(&treeView, &model, &column);
	createHome (&label, &view, &home_table, &notebook);
	createAll (&label, &scrolled_window, &notebook, &treeView);
	createActive (&label, &scrolled_window, &notebook, &treeView);
	createCompleted (&label, &scrolled_window, &notebook, &treeView);*/

// MOTD
	MOTD (&label, &table);

// Download/Upload speed stats
	duStats (&label, &table);

// Menu
	createMenu (&toolbar, &table);

// Show window widget and it's child widgets
	gtk_widget_show_all(window);

	g_signal_connect_swapped(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

	gtk_main ();

	return 0;
}