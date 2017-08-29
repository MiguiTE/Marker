#include <gtksourceview/gtksource.h>
#include <dirent.h>

#include "marker-editor-window.h"
#include "marker-utils.h"

static void
css_chosen(GtkComboBox* combo_box,
           gpointer     user_data)
{
    GtkApplication* app = user_data;
    char* css_theme = marker_combo_box_get_active_str(combo_box);
    
    GList* windows = gtk_application_get_windows(app);
    if (windows)
    {
        for (; windows != NULL; windows = windows->next)
        {
            MarkerEditorWindow* window = MARKER_EDITOR_WINDOW(windows->data);
            marker_editor_window_set_css_theme(window, css_theme);
        }
    }
}

static void
syntax_chosen(GtkComboBox* combo_box,
              gpointer     user_data)
{
    GtkApplication* app = user_data;
    char* syntax_theme = marker_combo_box_get_active_str(combo_box);
    
    GList* windows = gtk_application_get_windows(app);
    if (windows)
    {
        for (; windows != NULL; windows = windows->next)
        {
            MarkerEditorWindow* window = MARKER_EDITOR_WINDOW(windows->data);
            marker_editor_window_set_syntax_theme(window, syntax_theme);
        }
    }
}

static void
prefs_activated(GSimpleAction* action,
                GVariant*      parameter,
                gpointer       data)
{
    GtkApplication* app = data;
    GtkBuilder* builder = gtk_builder_new_from_resource("/com/github/fabiocolacio/marker/marker-prefs-window.ui");
    GtkWindow* win = GTK_WINDOW(gtk_builder_get_object(builder, "prefs_win"));
    
    GtkTreeIter iter;
    
    GtkListStore* syntax_list = gtk_list_store_new(1, G_TYPE_STRING);
    GtkSourceStyleSchemeManager* style_manager = gtk_source_style_scheme_manager_get_default();
    const gchar * const * ids = gtk_source_style_scheme_manager_get_scheme_ids(style_manager);
    for (int i = 0; ids[i] != NULL; ++i)
    {
        gtk_list_store_append(syntax_list, &iter);
        gtk_list_store_set(syntax_list, &iter, 0, ids[i], -1);
    }
    GtkComboBox* syntax_chooser = GTK_COMBO_BOX(gtk_builder_get_object(builder, "syntax_chooser"));
    gtk_combo_box_set_model(syntax_chooser, GTK_TREE_MODEL(syntax_list));
    GtkCellRenderer* cell_renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(syntax_chooser), cell_renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(syntax_chooser),
                                   cell_renderer,
                                   "text", 0,
                                   NULL);
    gtk_combo_box_set_active(syntax_chooser, 0);
    
    GtkListStore* css_list = gtk_list_store_new(1, G_TYPE_STRING);
    DIR* dir;
    struct dirent* ent;
    char* filename;
    if ((dir = opendir(STYLES_DIR)) != NULL)
    {
        while ((ent = readdir(dir)) != NULL)
        {
            filename = ent->d_name;
            if (marker_utils_str_ends_with(filename, ".css"))
            {
                gtk_list_store_append(css_list, &iter);
                gtk_list_store_set(css_list, &iter, 0, filename, -1);
            }
        }   
    }
    GtkComboBox* css_chooser = GTK_COMBO_BOX(gtk_builder_get_object(builder, "css_chooser"));
    gtk_combo_box_set_model(css_chooser, GTK_TREE_MODEL(css_list));
    cell_renderer = gtk_cell_renderer_text_new();
    gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(css_chooser), cell_renderer, TRUE);
    gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(css_chooser),
                                   cell_renderer,
                                   "text", 0,
                                   NULL);
    gtk_combo_box_set_active(css_chooser, 0);
    
    gtk_builder_add_callback_symbol(builder, "css_chosen", G_CALLBACK(css_chosen));
    gtk_builder_add_callback_symbol(builder, "syntax_chosen", G_CALLBACK(syntax_chosen));
    gtk_builder_connect_signals(builder, app);
    g_object_unref(builder);
    gtk_widget_show_all(GTK_WIDGET(win));
}

static void
about_activated(GSimpleAction* action,
                GVariant*      parameter,
                gpointer       data)
{

}

static void
quit_activated(GSimpleAction* action,
               GVariant*      parameter,
               gpointer       data)
{

}

static GActionEntry app_entries[] =
{
    { "prefs", prefs_activated, NULL, NULL, NULL },
    { "about", about_activated, NULL, NULL, NULL },
    { "quit", quit_activated, NULL, NULL, NULL }
};

static void
activate(GtkApplication* app)
{
    GtkBuilder* builder = gtk_builder_new_from_resource("/com/github/fabiocolacio/marker/marker-app-menu.ui");
    GMenuModel* app_menu = G_MENU_MODEL(gtk_builder_get_object(builder, "app_menu"));
    gtk_application_set_app_menu(app, app_menu);
    g_object_unref(builder);
    
    g_action_map_add_action_entries(G_ACTION_MAP(app),
                                    app_entries,
                                    G_N_ELEMENTS(app_entries),
                                    app);

    MarkerEditorWindow* window = marker_editor_window_new(app);
    gtk_widget_show_all(GTK_WIDGET(window));
}

void
marker_open(GtkApplication* app,
            GFile**         files,
            gint            num_files,
            const gchar*    hint)
{
    for (int i = 0; i < num_files; ++i)
    {
        GFile* file = files[i];
        g_object_ref(file);
        MarkerEditorWindow* win = marker_editor_window_new_from_file(app, file);
        gtk_widget_show_all(GTK_WIDGET(win));
    }
}

int
main(int    argc,
     char** argv)
{
    GtkApplication* app = gtk_application_new("com.github.fabiocolacio.marker",
                                              G_APPLICATION_HANDLES_OPEN);
    
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    g_signal_connect(app, "open", G_CALLBACK(marker_open), NULL);
    
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    
    return status;
}

