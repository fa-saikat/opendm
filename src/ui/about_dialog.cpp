#include "about_dialog.hpp"
#include "gtk_util.hpp"
#include "ui_assets.hpp"
#include <gio/gio.h>

namespace opendm {

AboutDialog* AboutDialog::show(GtkWindow* parent) {
    auto* dlg = new AboutDialog(parent);
    dlg->setup_ui();
    return dlg;
}

AboutDialog::AboutDialog(GtkWindow* parent) : parent_(parent) {}

void AboutDialog::setup_ui() {
    dialog_ = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(dialog_), "About OpenDM");
    gtk_window_set_modal(GTK_WINDOW(dialog_), TRUE);
    if (parent_) {
        gtk_window_set_transient_for(GTK_WINDOW(dialog_), parent_);
    }
    gtk_window_set_default_size(GTK_WINDOW(dialog_), 480, 360);
    gtk_window_set_resizable(GTK_WINDOW(dialog_), FALSE);
    gtk_window_set_deletable(GTK_WINDOW(dialog_), TRUE);
    ui::add_class(dialog_, "idm-about-window");

    g_signal_connect_swapped(dialog_, "destroy", G_CALLBACK(+[](AboutDialog* self) {
        delete self;
    }), this);

    GtkWidget* main_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 16);
    ui::add_class(main_vbox, "idm-about-window");
    gtk_widget_set_margin_start(main_vbox, 24);
    gtk_widget_set_margin_end(main_vbox, 24);
    gtk_widget_set_margin_top(main_vbox, 22);
    gtk_widget_set_margin_bottom(main_vbox, 22);
    gtk_container_add(GTK_CONTAINER(dialog_), main_vbox);

    // 1. App Header with Icon & Title
    GtkWidget* header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 16);
    GtkWidget* icon = UiAssets::create_image("folder-download-symbolic", 48, 48, "emblem-default-symbolic");
    ui::add_class(icon, "idm-about-icon");
    gtk_container_add(GTK_CONTAINER(header_box), icon);

    GtkWidget* title_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    GtkWidget* lbl_app = gtk_label_new(nullptr);
    gtk_label_set_markup(GTK_LABEL(lbl_app), "<span size='xx-large' weight='bold'>OpenDM</span>");
    gtk_label_set_xalign(GTK_LABEL(lbl_app), 0.0f);
    gtk_container_add(GTK_CONTAINER(title_vbox), lbl_app);

    GtkWidget* lbl_version = gtk_label_new("Version 1.0.0 (Linux x86_64) • Native GTK3");
    ui::add_class(lbl_version, "idm-about-sub");
    gtk_label_set_xalign(GTK_LABEL(lbl_version), 0.0f);
    gtk_container_add(GTK_CONTAINER(title_vbox), lbl_version);

    gtk_container_add(GTK_CONTAINER(header_box), title_vbox);
    gtk_container_add(GTK_CONTAINER(main_vbox), header_box);

    // 2. Description
    GtkWidget* lbl_desc = gtk_label_new(
        "High-performance multi-threaded internet download manager for Linux. "
        "Engineered for maximum bandwidth saturation, segmented downloads, dynamic speed limiting, and seamless browser integration."
    );
    gtk_label_set_line_wrap(GTK_LABEL(lbl_desc), TRUE);
    // Cap natural width: GTK3 sizes fixed-size windows to the unwrapped text otherwise.
    gtk_label_set_max_width_chars(GTK_LABEL(lbl_desc), 60);
    gtk_label_set_xalign(GTK_LABEL(lbl_desc), 0.0f);
    ui::add_class(lbl_desc, "idm-about-desc");
    gtk_container_add(GTK_CONTAINER(main_vbox), lbl_desc);

    // 3. Author & Credits Card
    GtkWidget* card = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    ui::add_class(card, "idm-about-card");
    gtk_container_add(GTK_CONTAINER(main_vbox), card);

    GtkWidget* dev_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget* dev_info = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    gtk_widget_set_hexpand(dev_info, TRUE);

    GtkWidget* lbl_dev = gtk_label_new(nullptr);
    gtk_label_set_markup(GTK_LABEL(lbl_dev), "<span weight='bold' size='large'>Nader Mahbub Khan</span>");
    gtk_label_set_xalign(GTK_LABEL(lbl_dev), 0.0f);
    gtk_container_add(GTK_CONTAINER(dev_info), lbl_dev);

    GtkWidget* lbl_role = gtk_label_new("Software Engineer • Author & Maintainer");
    ui::add_class(lbl_role, "idm-about-role");
    gtk_label_set_xalign(GTK_LABEL(lbl_role), 0.0f);
    gtk_container_add(GTK_CONTAINER(dev_info), lbl_role);

    GtkWidget* lbl_link = gtk_label_new("github.com/nadermkhan/");
    ui::add_class(lbl_link, "idm-about-link");
    gtk_label_set_xalign(GTK_LABEL(lbl_link), 0.0f);
    gtk_container_add(GTK_CONTAINER(dev_info), lbl_link);

    gtk_container_add(GTK_CONTAINER(dev_row), dev_info);

    GtkWidget* btn_gh = gtk_button_new_with_label("GitHub Profile ↗");
    gtk_widget_set_valign(btn_gh, GTK_ALIGN_CENTER);
    g_signal_connect_swapped(btn_gh, "clicked", G_CALLBACK(+[](AboutDialog* self) {
        self->on_github_clicked();
    }), this);
    gtk_container_add(GTK_CONTAINER(dev_row), btn_gh);

    gtk_container_add(GTK_CONTAINER(card), dev_row);

    // 4. License & Copyright
    GtkWidget* license_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget* lbl_license = gtk_label_new("Free & Open Source Software • GPL-3.0 License • 100% Free Forever");
    ui::add_class(lbl_license, "idm-about-license");
    gtk_label_set_xalign(GTK_LABEL(lbl_license), 0.0f);
    gtk_container_add(GTK_CONTAINER(license_box), lbl_license);
    gtk_container_add(GTK_CONTAINER(main_vbox), license_box);

    // 5. Bottom Actions
    GtkWidget* btn_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_margin_top(btn_box, 8);
    gtk_container_add(GTK_CONTAINER(main_vbox), btn_box);

    GtkWidget* spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_widget_set_hexpand(spacer, TRUE);
    gtk_container_add(GTK_CONTAINER(btn_box), spacer);

    GtkWidget* btn_close = gtk_button_new_with_label("Close");
    ui::add_class(btn_close, "suggested-action");
    ui::add_class(btn_close, "idm-btn-about-close");
    g_signal_connect_swapped(btn_close, "clicked", G_CALLBACK(+[](AboutDialog* self) {
        self->on_close_clicked();
    }), this);
    gtk_container_add(GTK_CONTAINER(btn_box), btn_close);

    ui::present(dialog_);
}

void AboutDialog::on_close_clicked() {
    gtk_widget_destroy(GTK_WIDGET(dialog_));
}

void AboutDialog::on_github_clicked() {
    g_app_info_launch_default_for_uri("https://github.com/nadermkhan/", nullptr, nullptr);
}

bool AboutDialog::save_snapshot(const std::string& filename) {
    if (!dialog_) return false;
    return ui::save_widget_png(dialog_, filename, 480, 360);
}

} // namespace opendm
