#pragma once

// Small GTK3 helpers shared across the UI layer.

#include <gtk/gtk.h>
#include <string>

namespace opendm::ui {

inline void add_class(GtkWidget* w, const char* css_class) {
    gtk_style_context_add_class(gtk_widget_get_style_context(w), css_class);
}

inline void remove_class(GtkWidget* w, const char* css_class) {
    gtk_style_context_remove_class(gtk_widget_get_style_context(w), css_class);
}

// Destroys every child of a GtkContainer.
inline void clear_container(GtkWidget* container) {
    GList* children = gtk_container_get_children(GTK_CONTAINER(container));
    for (GList* l = children; l != nullptr; l = l->next) {
        gtk_widget_destroy(GTK_WIDGET(l->data));
    }
    g_list_free(children);
}

// GtkComboBoxText filled from a nullptr-terminated list; first entry selected
// (matches GtkDropDown's default selection in GTK4).
inline GtkWidget* combo_new_from_strings(const char* const* items) {
    GtkWidget* combo = gtk_combo_box_text_new();
    for (int i = 0; items && items[i] != nullptr; ++i) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), items[i]);
    }
    if (items && items[0]) gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    return combo;
}

// Wraps a window-less widget (box, label, ...) in an input-only GtkEventBox so
// it can receive "button-press-event". CSS classes stay on the wrapped child.
inline GtkWidget* wrap_clickable(GtkWidget* child) {
    GtkWidget* ebox = gtk_event_box_new();
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(ebox), FALSE);
    gtk_widget_add_events(ebox, GDK_BUTTON_PRESS_MASK);
    gtk_container_add(GTK_CONTAINER(ebox), child);
    return ebox;
}

// Shows a toplevel and all of its descendants, then raises it.
// Widgets flagged with gtk_widget_set_no_show_all() keep their own visibility.
inline void present(GtkWidget* window) {
    gtk_widget_show_all(window);
    gtk_window_present(GTK_WINDOW(window));
}

// Renders a realized widget into a PNG file. Used by the --screenshot option.
inline bool save_widget_png(GtkWidget* widget, const std::string& filename,
                            int fallback_w, int fallback_h) {
    if (!widget || !gtk_widget_get_realized(widget)) return false;

    int w = gtk_widget_get_allocated_width(widget);
    int h = gtk_widget_get_allocated_height(widget);
    if (w <= 1) w = fallback_w;
    if (h <= 1) h = fallback_h;

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
    cairo_t* cr = cairo_create(surface);
    gtk_widget_draw(widget, cr);
    cairo_destroy(cr);

    bool ok = cairo_surface_write_to_png(surface, filename.c_str()) == CAIRO_STATUS_SUCCESS;
    cairo_surface_destroy(surface);
    return ok;
}

} // namespace opendm::ui
