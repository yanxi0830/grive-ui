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
	GtkWidget *upload_combo;
	GtkWidget *download_combo;
	GtkWidget *progress_bar;
} AppData;

const gchar *AUTH_URL =
		"https://accounts.google.com/o/oauth2/auth?scope=https%3A%2F%2Fwww.googleapis.com%2Fauth%2Fuserinfo.email+https%3A%2F%2Fwww.googleapis.com%2Fauth%2Fuserinfo.profile+https%3A%2F%2Fdocs.google.com%2Ffeeds%2F+https%3A%2F%2Fdocs.googleusercontent.com%2F+https%3A%2F%2Fspreadsheets.google.com%2Ffeeds%2F&redirect_uri=urn:ietf:wg:oauth:2.0:oob&response_type=code&client_id=22314510474.apps.googleusercontent.com";

static void show_dir_chooser(GtkWidget *button, gpointer data) {
	GtkWidget *dir_chooser_dialog;
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;
	gint res;
	GtkWidget *message_view = ((AppData *) data)->message_view;
	GtkWidget *progress_bar = ((AppData *) data)->progress_bar;


	dir_chooser_dialog = gtk_file_chooser_dialog_new(("Choose Google Drive Directory"),
													  GTK_WINDOW(((AppData * ) data)->window),
													  action,
													  ("_Cancel"),
													  GTK_RESPONSE_CANCEL,
													  ("_Open"),
													  GTK_RESPONSE_ACCEPT,
													  NULL);

	res = gtk_dialog_run(GTK_DIALOG(dir_chooser_dialog));

	if (res == GTK_RESPONSE_ACCEPT) {
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dir_chooser_dialog));
		gtk_label_set_text(GTK_LABEL(((AppData * )data)->curr_dir), filename);
		gtk_label_set_text(GTK_LABEL(message_view), "Click Sync to sync with Google Drive\n");
		gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.0);
		g_free(filename);
	}

	gtk_widget_destroy(dir_chooser_dialog);
}


static void about_activate(GtkWidget *widget, gpointer data) {
	/* Open About Dialog */
	GtkWidget *about_dialog;
	gchar cmd[1024];
	const gchar *description = "Grive is a Google Drive client for GNU/Linux systems.\n It allows the synchronization of all your files on the cloud with a directory of your choice and the upload of new files to Google Drive.";

	about_dialog = gtk_about_dialog_new();
	gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG(about_dialog), "Grive");

	/* Get the version */
	g_snprintf(cmd, 1024, "grive -v");
	FILE *grive = popen(cmd, "r");
	char buf[1024];
	while (fgets(buf, sizeof(buf), grive) != 0) {
		printf("%s", buf);
	}
	pclose(grive);

	gtk_about_dialog_set_version (GTK_ABOUT_DIALOG(about_dialog), buf);
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog), description);

	gtk_show_about_dialog(GTK_WINDOW(((AppData * ) data)->window),
						  "program-name", "Grive",
						  "version", buf,
						  "comments", description,
						  "title", "About Grive",
						  NULL);

}

static void help_contents_activate(GtkWidget *widget, gpointer data) {
	/* Open Help Dialog */
}

static void auth_cb(GtkWidget *button, gpointer data) {
	GtkWidget *auth_dialog;
	GtkWidget *auth_content, *auth_vbox, *url_hbox, *token_hbox;
	GtkWidget *url_help_label, *url_label, *token_help_label;
	GtkWidget *token_entry;
	const gchar *token_text;
	gint res;
	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	GtkWidget *message_view = ((AppData *) data)->message_view;

	const gchar *sync_dir;
	gchar cmd[2048];
	GtkWidget *sync_label = ((AppData *) data)->curr_dir;

	sync_dir = gtk_label_get_text(GTK_LABEL(sync_label));

	g_snprintf(cmd, 2048, "grive -p %s -a", sync_dir);

	/* Authorize, run grive in command line */
	FILE *grive = popen(cmd, "w");

	/* Open a dialog to display URL, enter token */
	auth_dialog = gtk_dialog_new_with_buttons("Authentication",
											  GTK_WINDOW(((AppData * ) data)->window),
											  flags,
											  ("_OK"),
											  GTK_RESPONSE_ACCEPT,
											  NULL);

	auth_content = gtk_dialog_get_content_area(GTK_DIALOG(auth_dialog));
	auth_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	gtk_container_add(GTK_CONTAINER(auth_content), auth_vbox);

	url_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	token_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_pack_start(GTK_BOX(auth_vbox), url_hbox, TRUE, TRUE, 10);
	gtk_box_pack_start(GTK_BOX(auth_vbox), token_hbox, TRUE, TRUE, 10);

	url_help_label = gtk_label_new("Please go to this URL and get an authentication code: ");
	gtk_box_pack_start(GTK_BOX(url_hbox), url_help_label, FALSE, TRUE, 10);
	url_label = gtk_link_button_new_with_label(AUTH_URL, "Get Authentication Code");
	gtk_box_pack_start(GTK_BOX(url_hbox), url_label, FALSE, TRUE, 0);
	token_help_label = gtk_label_new("Please input the authentication code here: ");
	gtk_box_pack_start(GTK_BOX(token_hbox), token_help_label, FALSE, TRUE, 10);

	/* Token Entry */
	token_entry = gtk_entry_new();
	gtk_box_pack_start(GTK_BOX(token_hbox), token_entry, TRUE, TRUE, 0);

	gtk_widget_show_all(auth_dialog);

	res = gtk_dialog_run(GTK_DIALOG(auth_dialog));

	switch (res) {
		case GTK_RESPONSE_ACCEPT:
			token_text = gtk_entry_get_text(GTK_ENTRY(token_entry));
			printf("Entered: %s\n", token_text);
			fputs(token_text, grive);
			pclose(grive);
			gtk_label_set_text(GTK_LABEL(message_view), "Click Sync to sync with Google Drive\n");
			break;
		default:
			pclose(grive);
	}

	gtk_widget_destroy(auth_dialog);
}

static gboolean fill (gpointer data) {
	GtkWidget *progress_bar = ((AppData *) data)->progress_bar;

	/*Get the current progress*/
	gdouble fraction;
	fraction = gtk_progress_bar_get_fraction (GTK_PROGRESS_BAR (progress_bar));

	/*Increase the bar by 10% each time this function is called*/
	fraction += 0.1;

	/*Fill in the bar with the new fraction*/
	gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (progress_bar), fraction);

	if (fraction < 1.0) {
		return TRUE;
	}

	return FALSE;

}

static void sync_cb(GtkWidget *button, gpointer data) {
	const gchar* sync_dir;
	gchar cmd[2048];
	GtkWidget *message_view = ((AppData *) data)->message_view;
	GtkWidget *sync_label = ((AppData *) data)->curr_dir;
	GtkWidget *force_check = ((AppData *) data)->force_check;
	GtkWidget *upload_check = ((AppData *) data)->upload_check;
	GtkWidget *no_remote_check = ((AppData *) data)->no_remote_check;
	GtkWidget *upload_combo = ((AppData *) data)->upload_combo;
	GtkWidget *download_combo = ((AppData *) data)->download_combo;
	GtkWidget *progress_bar = ((AppData *) data)->progress_bar;
	gchar *upload_speed, *download_speed;

	gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.0);
	g_timeout_add (100, fill, data);

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

	upload_speed = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(upload_combo));
	if (atoi(upload_speed) > 0) {
		strcat(cmd, " -U ");
		strcat(cmd, upload_speed);
	}
	g_free(upload_speed);

	download_speed = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(download_combo));
	if (atoi(download_speed) > 0) {
		strcat(cmd, " -D ");
		strcat(cmd, download_speed);
	}
	g_free(download_speed);

	strcat(cmd, " 2>&1");

	printf("COMMAND %s\n", cmd);

	/* Sync, run grive in command line */
	FILE *grive = popen(cmd, "r");
	char buf[1024];
	int i = 0;
	while (fgets(buf, sizeof(buf), grive) != 0) {
		printf("%d %s", i, buf);
		i++;
	}
	pclose(grive);

	if (i == 1) {
		gtk_label_set_text(GTK_LABEL(message_view),
				"Please click Authenticate if this is the first time you're accessing your Google Drive!\n");
	} else {
		gtk_label_set_text(GTK_LABEL(message_view), buf);
	}

}


static void activate(GtkApplication* app, gpointer user_data) {
	GtkWidget *main_vbox, *choose_dir_hbox, *sync_hbox, *check_hbox;
	GtkWidget *choose_dir_button, *choose_dir_label;
	GtkWidget *check_label;
	GtkWidget *quit_button, *sync_button, *auth_button;
	GtkWidget *menu_bar, *help_menu, *help, *help_contents, *about;
	GtkWidget *message_frame, *message_hbox;
	GtkWidget *speed_vbox, *speed_hbox, *speed_frame, *speed_hbox_in, *upload_label, *download_label;

	AppData *app_data = g_new0(AppData, 1);
	int i;

	/* Top-level Window */
	app_data->window = gtk_application_window_new(app);
	gtk_window_set_title(GTK_WINDOW(app_data->window), "Grive");

	/* Box for packing */
	main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(app_data->window), main_vbox);

	/* Menu Bar */
	menu_bar = gtk_menu_bar_new();

	help_menu = gtk_menu_new();
	help = gtk_menu_item_new_with_label("Help");
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), help_menu);

	help_contents = gtk_menu_item_new_with_label("Help Contents");
	g_signal_connect(help_contents, "activate", G_CALLBACK(help_contents_activate), app_data);
	gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), help_contents);

	about = gtk_menu_item_new_with_label("About");
	g_signal_connect(about, "activate", G_CALLBACK(about_activate), app_data);
	gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), about);

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

	/* Speed Limit */
	speed_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_pack_start(GTK_BOX(main_vbox), speed_hbox, FALSE, TRUE, 10);
	speed_frame = gtk_frame_new("Speed Limit");
	gtk_box_pack_start(GTK_BOX(speed_hbox), speed_frame, TRUE, TRUE, 10);
	app_data->upload_combo = gtk_combo_box_text_new_with_entry();
	app_data->download_combo = gtk_combo_box_text_new_with_entry();
	speed_hbox_in = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	speed_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
	gtk_box_pack_start(GTK_BOX(speed_vbox), speed_hbox_in, TRUE, TRUE, 10);
	gtk_container_add(GTK_CONTAINER(speed_frame), speed_vbox);

	upload_label = gtk_label_new("Upload Speed");
	download_label = gtk_label_new("Download Speed");

	gtk_box_pack_start(GTK_BOX(speed_hbox_in), upload_label, TRUE, TRUE, 10);
	gtk_box_pack_start(GTK_BOX(speed_hbox_in), app_data->upload_combo, TRUE, TRUE, 10);
	gtk_box_pack_start(GTK_BOX(speed_hbox_in), download_label, TRUE, TRUE, 10);
	gtk_box_pack_start(GTK_BOX(speed_hbox_in), app_data->download_combo, TRUE, TRUE, 10);

	const char *speeds[] = {"Unlimited", "100", "200", "300", "500", "1000"};

	for (i = 0; i < G_N_ELEMENTS(speeds); i++) {
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app_data->upload_combo), speeds[i]);
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(app_data->download_combo), speeds[i]);
	}

	gtk_combo_box_set_active(GTK_COMBO_BOX(app_data->upload_combo), 0);
	gtk_combo_box_set_active(GTK_COMBO_BOX(app_data->download_combo), 0);

	/* Message View / Progress Bar */
	message_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
	gtk_box_pack_start(GTK_BOX(main_vbox), message_hbox, FALSE, TRUE, 10);
	message_frame = gtk_frame_new("Message");
	app_data->message_view = gtk_label_new("Click Sync to sync with Google Drive\n");
	gtk_container_add(GTK_CONTAINER(message_frame), app_data->message_view);
	gtk_box_pack_start(GTK_BOX(message_hbox), message_frame, TRUE, TRUE, 10);

	app_data->progress_bar = gtk_progress_bar_new();
	gtk_box_pack_start(GTK_BOX(main_vbox), app_data->progress_bar, TRUE, TRUE, 10);
	// g_timeout_add (100, fill, app_data);

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

	auth_button = gtk_button_new_with_label("Authenticate");
	gtk_box_pack_start(GTK_BOX(sync_hbox), auth_button, FALSE, TRUE, 10);
	g_signal_connect(auth_button, "clicked", G_CALLBACK(auth_cb), app_data);


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
