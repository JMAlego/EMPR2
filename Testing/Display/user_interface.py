#!/usr/bin/env python3

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, GObject, Pango
from multiprocessing import Process, Value, Array
from ui_channel_display import UIChannelDisplay

class DisplayUI(Gtk.Window):

    def __init__(self):
        self.packet_count_value = Value("i", 0)
        self.ui_running = Value("b", 1)
        self.reset_count = Value("b", 0)
        self.packet_last_value = Array("B", list(range(512)))
        self.packet_snapshot = self.packet_last_value[:]
        self.mode = "SINGLE_CAPTURE"
        Gtk.Window.__init__(self, title="EMPR PC Display")
        self.set_default_size(800, 600)
        self.set_border_width(10)
        self.channel_windows = {}

        hbox = Gtk.Box(spacing=10)
        hbox.set_homogeneous(False)
        vbox_left = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=10)
        vbox_left.set_homogeneous(False)
        vbox_right = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=10)
        vbox_right.set_homogeneous(False)

        hbox.pack_start(vbox_left, True, True, 0)
        hbox.pack_start(vbox_right, True, True, 0)

        label = Gtk.Label("Packet Count: 0")
        label.set_line_wrap(True)
        vbox_right.pack_start(label, False, True, 0)
        self.packet_count_label = label

        scrolledwindow = Gtk.ScrolledWindow()

        liststore = Gtk.ListStore(str, str)
        self.packet_data = liststore
        for i in range(512):
            liststore.append([str(i+1), "0x00"])

        treeview = Gtk.TreeView(model=liststore)
        fontdesc = Pango.FontDescription("monospace 10")
        treeview.modify_font(fontdesc)
        renderer_slot = Gtk.CellRendererText()
        column_slot = Gtk.TreeViewColumn("Slot", renderer_slot, text=0)
        treeview.append_column(column_slot)
        renderer_value = Gtk.CellRendererText()
        column_value = Gtk.TreeViewColumn("Value", renderer_value, text=1)
        treeview.append_column(column_value)
        self.packet_view = treeview

        scrolledwindow.add(treeview)
        vbox_right.pack_start(scrolledwindow, True, True, 0)

        label = Gtk.Label("Menu")
        vbox_left.pack_start(label, False, True, 0)

        button = Gtk.Button(label="Reset Count")
        button.connect("clicked", self.btn_reset_count)
        label.set_mnemonic_widget(button)
        vbox_left.pack_start(button, False, True, 0)

        button = Gtk.Button(label="Single Capture")
        button.connect("clicked", self.btn_single_capture)
        label.set_mnemonic_widget(button)
        vbox_left.pack_start(button, False, True, 0)

        button = Gtk.Button(label="Continuous Capture")
        button.connect("clicked", self.btn_multi_capture)
        label.set_mnemonic_widget(button)
        vbox_left.pack_start(button, False, True, 0)

        channel_monitor_frame = Gtk.Frame(label="Channel Monitor")

        channel_monitor_box = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=10)
        channel_monitor_box.set_border_width(10)

        channel_sizes_store = Gtk.ListStore(int, str)
        channel_sizes_store.append([3, "3 Slot Lighting Channel"])
        channel_sizes_store.append([7, "7 Slot Lighting Channel"])
        channel_sizes_store.append([8, "8 Slot Lighting Channel"])

        channel_sizes_combo = Gtk.ComboBox.new_with_model_and_entry(channel_sizes_store)
        self.channel_sizes_combo = channel_sizes_combo
        #name_combo.connect("changed", self.on_name_combo_changed)
        channel_sizes_combo.set_entry_text_column(1)
        channel_monitor_box.pack_start(channel_sizes_combo, False, False, 0)

        button = Gtk.Button(label="New Channel Monitor")
        button.connect("clicked", self.btn_create_channel_monitor)
        label.set_mnemonic_widget(button)
        channel_monitor_box.pack_start(button, False, True, 0)

        channel_monitor_frame.add(channel_monitor_box)
        vbox_left.pack_start(channel_monitor_frame, False, True, 0)

        self.add(hbox)

        self.timeout_id = GObject.timeout_add(100, self.on_timeout, None)

    def update_data(self, data):
        self.packet_data.get_buffer().set_text(data)

    def on_timeout(self, event=None):
        self.packet_count_label.set_text("Packet Count: " + str(self.packet_count_value.value))
        if self.mode == "SINGLE_CAPTURE":
            self.packet_snapshot = self.packet_last_value[:]
            index = 0
            for value in self.packet_snapshot:
                self.packet_data[index] = [str(index + 1), "0x" + hex(value)[2:].rjust(2, "0")]
                index += 1
            for cid, child in self.channel_windows.items():
                child.update_values()
            self.mode = "WAITING"
        if self.mode == "MULTI_CAPTURE":
            self.packet_snapshot = self.packet_last_value[:]
            index = 0
            for value in self.packet_snapshot:
                self.packet_data[index] = [str(index + 1), "0x" + hex(value)[2:].rjust(2, "0")]
                index += 1
            for cid, child in self.channel_windows.items():
                child.update_values()
        return True

    def btn_reset_count(self, event):
        self.reset_count.value = 1

    def btn_single_capture(self, event):
        self.mode = "SINGLE_CAPTURE"

    def btn_multi_capture(self, event):
        self.mode = "MULTI_CAPTURE"

    def btn_create_channel_monitor(self, event):
        channel_size = 3
        start_slot = 1
        active = self.packet_view.get_selection().get_selected()[1]
        if active is not None:
            start_slot = int(self.packet_view.get_model()[active][0])
        active = self.channel_sizes_combo.get_active_iter()
        if active is not None:
            model = self.channel_sizes_combo.get_model()
            row_id, name = model[active][:2]
            channel_size = row_id
        elif self.channel_sizes_combo.get_child().get_text() != "":
            entry_text = self.channel_sizes_combo.get_child().get_text()
            entry_value = 0
            try:
                channel_size = int(entry_text)
            except:
                channel_size = 3
        if not 513 > channel_size > 0:
            channel_size = 3
        new_window = UIChannelDisplay(self, start_slot, channel_size)
        self.channel_windows[new_window.id] = new_window

    def on_quit(self, event=None, event2=None):
        self.ui_running.value = 0
        Gtk.main_quit()

    @staticmethod
    def create_ui(win):
        win.ui_running.value = 1
        win.connect("delete-event", win.on_quit)
        win.show_all()
        Gtk.main()

if __name__ == "__main__":
    print("This module will not function properly if run, please run the core display file instead.")
    win = DisplayUI()
    DisplayUI.create_ui(win)
