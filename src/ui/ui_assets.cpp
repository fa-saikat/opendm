#include "ui_assets.hpp"
#include "gtk_util.hpp"
#include <filesystem>
#include <iostream>
#include <vector>
#include <unistd.h>

namespace fs = std::filesystem;

namespace opendm {

std::string UiAssets::get_path(const std::string& filename) {
    std::vector<std::string> subdirs = {"", "neon/"};

    for (const auto& sub : subdirs) {
        std::string candidate = sub + filename;

        // 1. Current working directory data/ui_assets/
        if (fs::exists("data/ui_assets/" + candidate)) {
            return fs::absolute("data/ui_assets/" + candidate).string();
        }

        // 2. Relative to executable path (/proc/self/exe)
        char self_path[4096];
        ssize_t len = readlink("/proc/self/exe", self_path, sizeof(self_path) - 1);
        if (len > 0) {
            self_path[len] = '\0';
            fs::path exe_dir = fs::path(self_path).parent_path();
            
            fs::path c1 = exe_dir / "data" / "ui_assets" / candidate;
            if (fs::exists(c1)) return c1.string();

            fs::path c2 = exe_dir.parent_path() / "data" / "ui_assets" / candidate;
            if (fs::exists(c2)) return c2.string();
        }

        // 3. User local share ~/.local/share/opendm/ui_assets/
        const char* home = getenv("HOME");
        if (home) {
            fs::path user_share = fs::path(home) / ".local" / "share" / "opendm" / "ui_assets" / candidate;
            if (fs::exists(user_share)) return user_share.string();
        }

        // 4. System share /usr/share/opendm/ui_assets/
        fs::path sys_share = fs::path("/usr/share/opendm/ui_assets") / candidate;
        if (fs::exists(sys_share)) return sys_share.string();
    }

    return "";
}

GtkWidget* UiAssets::create_image(
    const std::string& name_or_file,
    int width,
    int height,
    const char* fallback_icon) {

    int target_size = (width > 0) ? width : 20;

    // 1. If it has a file extension or slash, try to load from assets
    if (name_or_file.find('/') != std::string::npos || name_or_file.find(".png") != std::string::npos || name_or_file.find(".svg") != std::string::npos) {
        std::string full_path = get_path(name_or_file);
        if (!full_path.empty()) {
            // GTK3 only applies pixel-size to themed icons, so scale file assets on load.
            if (width > 0) {
                int h = (height > 0) ? height : width;
                GdkPixbuf* pb = gdk_pixbuf_new_from_file_at_scale(full_path.c_str(), width, h, TRUE, nullptr);
                if (pb) {
                    GtkWidget* img = gtk_image_new_from_pixbuf(pb);
                    g_object_unref(pb);
                    return img;
                }
            }
            return gtk_image_new_from_file(full_path.c_str());
        }
    }

    // 2. Otherwise treat name_or_file as GTK icon name
    GtkWidget* img = gtk_image_new_from_icon_name(name_or_file.c_str(), GTK_ICON_SIZE_BUTTON);
    gtk_image_set_pixel_size(GTK_IMAGE(img), target_size);
    return img;
}

GtkWidget* UiAssets::create_toolbar_button(
    const std::string& icon_name,
    const std::string& label_text,
    const std::string& tooltip_text,
    GCallback callback,
    gpointer user_data,
    bool is_primary,
    bool has_dropdown) {

    GtkWidget* btn = gtk_button_new();
    ui::make_flat(btn);
    ui::add_class(btn, "idm-toolbar-btn");
    if (is_primary) {
        ui::add_class(btn, "suggested-action");
    }
    gtk_widget_set_tooltip_text(btn, tooltip_text.c_str());

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_margin_start(box, 4);
    gtk_widget_set_margin_end(box, 4);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

    GtkWidget* img = create_image(icon_name, 22, 22, "document-save-symbolic");
    gtk_widget_set_halign(img, GTK_ALIGN_CENTER);
    gtk_container_add(GTK_CONTAINER(box), img);

    GtkWidget* lbl_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 2);
    gtk_widget_set_halign(lbl_box, GTK_ALIGN_CENTER);

    GtkWidget* lbl = gtk_label_new(label_text.c_str());
    ui::add_class(lbl, "idm-toolbar-label");
    gtk_container_add(GTK_CONTAINER(lbl_box), lbl);

    if (has_dropdown) {
        GtkWidget* chevron = gtk_label_new("▾");
        ui::add_class(chevron, "idm-toolbar-chevron");
        gtk_container_add(GTK_CONTAINER(lbl_box), chevron);
    }

    gtk_container_add(GTK_CONTAINER(box), lbl_box);
    gtk_container_add(GTK_CONTAINER(btn), box);

    if (callback) {
        g_signal_connect(btn, "clicked", callback, user_data);
    }

    return btn;
}

void UiAssets::apply_theme() {
    std::string css_path = "";
    
    std::vector<fs::path> candidates;
    // 1. Current working directory
    candidates.push_back("src/ui/style.css");
    candidates.push_back("data/ui_assets/style.css");

    // 2. User local share ~/.local/share/opendm/style.css
    const char* home = getenv("HOME");
    if (home) {
        candidates.push_back(fs::path(home) / ".local" / "share" / "opendm" / "style.css");
    }

    // 3. Executable-relative paths
    char self_path[4096];
    ssize_t len = readlink("/proc/self/exe", self_path, sizeof(self_path) - 1);
    if (len > 0) {
        self_path[len] = '\0';
        fs::path exe_dir = fs::path(self_path).parent_path();
        candidates.push_back(exe_dir / "style.css");
        candidates.push_back(exe_dir / "share" / "opendm" / "style.css");
        candidates.push_back(exe_dir.parent_path() / "share" / "opendm" / "style.css");
        candidates.push_back(exe_dir.parent_path() / "src" / "ui" / "style.css");
        candidates.push_back(exe_dir.parent_path().parent_path() / "src" / "ui" / "style.css");
    }

    // 4. System paths
    candidates.push_back("/usr/local/share/opendm/style.css");
    candidates.push_back("/usr/share/opendm/style.css");

    for (const auto& p : candidates) {
        if (fs::exists(p)) {
            css_path = fs::absolute(p).string();
            break;
        }
    }

    if (g_getenv("OPENDM_DEBUG")) {
        std::cerr << "OpenDM: stylesheet "
                  << (css_path.empty() ? std::string("<built-in fallback>") : css_path) << std::endl;
    }

    GtkCssProvider* provider = gtk_css_provider_new();
    if (!css_path.empty()) {
        GError* css_err = nullptr;
        if (!gtk_css_provider_load_from_path(provider, css_path.c_str(), &css_err)) {
            std::cerr << "OpenDM: failed to load stylesheet " << css_path << ": "
                      << (css_err ? css_err->message : "unknown error") << std::endl;
        }
        if (css_err) g_error_free(css_err);
    } else {
        // Fallback when style.css is missing: layout-only rules that use the
        // theme's own colours, like style.css itself.
        static const char* fallback_css = R"(
            .idm-sidebar-frame { border-right: 1px solid @borders; }
            .idm-table-header { padding: 6px 12px; border-bottom: 1px solid @borders; }
            .idm-tree-row, .idm-download-row { padding: 4px 8px; }
            .idm-tree-row.selected, .idm-download-row.selected {
                background-color: @theme_selected_bg_color; color: @theme_selected_fg_color; }
            .idm-col-header { font-weight: bold; }
            .idm-statusbar { padding: 2px 8px; border-top: 1px solid @borders; }
        )";
        gtk_css_provider_load_from_data(provider, fallback_css, -1, nullptr);
    }

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

} // namespace opendm
