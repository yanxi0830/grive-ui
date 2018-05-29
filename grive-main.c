#include <gtk/gtk.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

/* contains all the widgets that may have to be changed dynamically */
typedef struct AppData {
	GtkWidget *window;
	GtkWidget *curr_dir;
	GtkWidget *message_view;
	GtkWidget *force_check;
	GtkWidget *upload_check;
	GtkWidget *no_remote_check;
} AppData;

static void show_dir_chooser(GtkWidget *button, gpointer data) {
	GtkWidget *dir_chooser_dialog;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
	gint res;
	GtkWidget *message_view = ((AppData *) data)->message_view;

	dir_chooser_dialog = gtk_file_chooser_dialog_new(("Choose Google Drive Directory"),
													  GTK_WINDOW(((AppData * ) data)->window),
													  action,
													  ("_Cancel"),
													  GTK_RESPONSE_CANCEL,
													  ("_Open"),
													  GTK_RESPONSE_ACCEPT,
													  NULL);

	res = gtk_dialog_run(GTK_DIALOG(dir_chooser_dialog));
	//gtk_widget_show_all(((AppData *)data)->window);

	if (res == GTK_RESPONSE_ACCEPT) {
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dir_chooser_dialog));
		gtk_label_set_text(GTK_LABEL(((AppData * )data)->curr_dir), filename);
		gtk_label_set_text(GTK_LABEL(message_view), "Click Sync to sync with Google Drive\n");
		g_free(filename);
	}

	gtk_widget_destroy(dir_chooser_dialog);
}

static void preferences_activate(GtkWidget *widget, gpointer data) {
	// TODO
	/* Open Preferences Dialog */
	GtkWidget *preference_dialog;
	GtkWidget *preference_content;
	GtkWidget *preference_vbox, *upload_hbox, *download_hbox, *upload_frame, *download_frame;
	GtkWidget *upload_combo, *download_combo;
	int i;

	gint res;
	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;

	preference_dialog = gtk_dialog_new_with_buttons("Preferences",
													GTK_WINDOW(((AppData * ) data)->window),
													flags,
													("_Cancel"),
													GTK_RESPONSE_REJECT,
													("_OK"),
													GTK_RESPONSE_ACCEPT,
													NULL);

	preference_content = gtk_dialog_get_content_area(GTK_DIALOG(preference_dialog));
	preference_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	upload_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	download_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	upload_frame = gtk_frame_new("Upload Speed");
	download_frame = gtk_frame_new("Download Speed");
	gtk_container_add(GTK_CONTAINER(preference_content), preference_vbox);
	gtk_box_pack_start(GTK_BOX(preference_vbox), upload_hbox, FALSE, TRUE, 10);
	gtk_box_pack_start(GTK_BOX(preference_vbox), download_hbox, FALSE, TRUE, 10);
	gtk_box_pack_start(GTK_BOX(upload_hbox), upload_frame, TRUE, TRUE, 10);
	gtk_box_pack_start(GTK_BOX(download_hbox), download_frame, TRUE, TRUE, 10);

	/* Combo box to select speeds */
	upload_combo = gtk_combo_box_text_new();
	download_combo = gtk_combo_box_text_new();
	gtk_container_add(GTK_CONTAINER(upload_frame), upload_combo);
	gtk_container_add(GTK_CONTAINER(download_frame), download_combo);
	const char *speeds[] = {"Unlimited", "100", "200", "300", "500", "1000"};

	for (i = 0; i < G_N_ELEMENTS(speeds); i++) {
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(upload_combo), speeds[i]);
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(download_combo), speeds[i]);
	}

	gtk_combo_box_set_active(GTK_COMBO_BOX(upload_combo), 0);
	gtk_combo_box_set_active(GTK_COMBO_BOX(download_combo), 0);

	gtk_widget_show_all(preference_dialog);

	res = gtk_dialog_run(GTK_DIALOG(preference_dialog));

	switch(res) {
		case GTK_RESPONSE_ACCEPT:
			break;
		default:
			break;
	}
	gtk_widget_destroy(preference_dialog);

}

static void about_activate(GtkWidget *widget, gpointer data) {
	// TODO
	/* Open About Dialog */
}

static void help_contents_activate(GtkWidget *widget, gpointer data) {
	// TODO
	/* Open Help Dialog */
}

static void sync_cb(GtkWidget *button, gpointer data) {
	const gchar* sync_dir;
	gchar cmd[2048];
	GtkWidget *message_view = ((AppData *) data)->message_view;
	GtkWidget *sync_label = ((AppData *) data)->curr_dir;
	GtkWidget *force_check = ((AppData *) data)->force_check;
	GtkWidget *upload_check = ((AppData *) data)->upload_check;
	GtkWidget *no_remote_check = ((AppData *) data)->no_remote_check;

	sync_dir = gtk_label_get_text(GTK_LABEL(sync_label));

	/* Accumulate the flags based on checkbox */
	g_snprintf(cmd, 2048, "grive -p %s", sync_dir);

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(force_check))) {
		strcat(cmd, " -f");
	}

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(upload_check))) {
		strcat(cmd, " -u");
	}

	if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(no_remote_check))) {
		strcat(cmd, " -n");
	}

	strcat(cmd, " 2>&1");

	printf("COMMAND %s\n", cmd);

	/* Sync, run grive in command line */
	FILE *grive = popen(cmd, "r");
	char buf[1024];
	int i = 0;
	while (fgets(buf, sizeof(buf), grive) != 0) {
		/* Show on GUI */
		printf("%d %s", i, buf);
		i++;
	}
	pclose(grive);
	gtk_label_set_text(GTK_LABEL(message_view), buf);

}

static void activate(GtkApplication* app, gpointer user_data) {
	GtkWidget *main_vbox, *choose_dir_hbox, *sync_hbox, *check_hbox;
	GtkWidget *choose_dir_button, *choose_dir_label;
	GtkWidget *check_label;
	GtkWidget *quit_button, *sync_button;
	GtkWidget *menu_bar, *help_menu, *settings_menu, *help, *settings, *help_contents, *about, *preferences;
	GtkWidget *message_frame, *message_hbox;

	AppData *app_data = g_new0(AppData, 1);

	/* Top-level Window */
	app_data->window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(app_data->window), "Grive2");
	// gtk_window_set_default_size(GTK_WINDOW(app_data->window), 500, 200);

	/* Box for packing */
	main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(app_data->window), main_vbox);

	/* Menu Bar */
	menu_bar = gtk_menu_bar_new();

	settings_menu = gtk_menu_new();
	settings = gtk_menu_item_new_with_label("Settings");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(settings), settings_menu);

	preferences = gtk_menu_item_new_with_label("Preferences");
	g_signal_connect(preferences, "activate", G_CALLBACK(preferences_activate), app_data);
	gtk_menu_shell_append(GTK_MENU_SHELL(settings_menu), preferences);

	help_menu = gtk_menu_new();
	help = gtk_menu_item_new_with_label("Help");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), help_menu);

	help_contents = gtk_menu_item_new_with_label("Help Contents");
	g_signal_connect(help_contents, "activate", G_CALLBACK(help_contents_activate), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), help_contents);

	about = gtk_menu_item_new_with_label("About");
	g_signal_connect(about, "activate", G_CALLBACK(about_activate), NULL);
	gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), about);

	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), settings);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help);

	gtk_box_pack_start(GTK_BOX(main_vbox), menu_bar, FALSE, FALSE,  0);

	/* Open File Chooser Dialog */
	choose_dir_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_pack_start(GTK_BOX(main_vbox), choose_dir_hbox, FALSE, TRUE, 10);

	// Get current working directory
	char cwd[1024];
	if (getcwd(cwd, sizeof(cwd)) == NULL) {
		perror("getcwd");
	}

	choose_dir_label = gtk_label_new("Google Drive Directory: ");
	gtk_box_pack_start(GTK_BOX(choose_dir_hbox), choose_dir_label, FALSE, FALSE, 10);

	app_data->curr_dir = gtk_label_new(cwd);

	gtk_box_pack_start(GTK_BOX(choose_dir_hbox), app_data->curr_dir, FALSE, FALSE, 0);
	choose_dir_button = gtk_button_new_with_label("Choose Directory");
	gtk_box_pack_end(GTK_BOX(choose_dir_hbox), choose_dir_button, FALSE, FALSE, 10);

	g_signal_connect(choose_dir_button, "clicked", G_CALLBACK(show_dir_chooser), app_data);

	/* Check Buttons */
	check_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_pack_start(GTK_BOX(main_vbox), check_hbox, FALSE, TRUE, 10);

	check_label = gtk_label_new("Options:");
	app_data->force_check = gtk_check_button_new_with_label("Force Download");
	app_data->upload_check = gtk_check_button_new_with_label("Upload Only");
	app_data->no_remote_check = gtk_check_button_new_with_label("No Remote New");
	gtk_box_pack_start(GTK_BOX(check_hbox), check_label, FALSE, TRUE, 10);
	gtk_box_pack_start(GTK_BOX(check_hbox), app_data->force_check, FALSE, TRUE, 10);
	gtk_box_pack_start(GTK_BOX(check_hbox), app_data->upload_check, FALSE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(check_hbox), app_data->no_remote_check, FALSE, TRUE, 10);

	/* Message View / Progress Bar */
	message_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_pack_start(GTK_BOX(main_vbox), message_hbox, FALSE, TRUE, 10);
	message_frame = gtk_frame_new("Message");
	app_data->message_view = gtk_label_new("Click Sync to sync with Google Drive\n");
	gtk_container_add(GTK_CONTAINER(message_frame), app_data->message_view);
	gtk_box_pack_start(GTK_BOX(message_hbox), message_frame, TRUE, TRUE, 10);

	/* Sync and Quit Button */
	sync_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_pack_end(GTK_BOX(main_vbox), sync_hbox, FALSE, TRUE, 10);

	sync_button = gtk_button_new_with_label("Sync");
	gtk_box_pack_end(GTK_BOX(sync_hbox), sync_button, FALSE, TRUE, 10);
	g_signal_connect(sync_button, "clicked", G_CALLBACK(sync_cb), app_data);

	quit_button = gtk_button_new_with_label("Quit");
	gtk_box_pack_end(GTK_BOX(sync_hbox), quit_button, FALSE, TRUE, 0);
	g_signal_connect_swapped(quit_button, "clicked",
							 G_CALLBACK(gtk_widget_destroy), app_data->window);


	gtk_widget_show_all(app_data->window);

}

int main(int argc, char *argv[]) {
	GtkApplication *app;
	int status;

	app = gtk_application_new("hello.world.app", G_APPLICATION_FLAGS_NONE);
	g_signal_connect(app, "activate", G_CALLBACK (activate), NULL);
	status = g_application_run(G_APPLICATION(app), argc, argv);

	g_object_unref(app);

	return status;
}
