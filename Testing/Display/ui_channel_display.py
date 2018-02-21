#!/usr/bin/env python3

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, Gdk, GObject, Pango

class UIChannelDisplay(Gtk.Window):
    channel_window_index = 0

    def __init__(self, parent, start_slot=1, channel_size=3):
        Gtk.Window.__init__(self, title="EMPR Display - Channel Viewer")
        self.parent = parent
        self.set_default_size(200,200)
        self.set_border_width(10)
        self.start_slot = start_slot
        self.channel_size = channel_size
        self.colour_label = None
        self.colour_area = None

        vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=10)
        vbox.set_homogeneous(False)

        label = Gtk.Label(label="Channel Data")
        vbox.pack_start(label, False, True, 0)

        scrolledwindow = Gtk.ScrolledWindow()

        liststore = Gtk.ListStore(str, str)
        self.packet_data = liststore
        for i in range(channel_size):
            liststore.append([str(i+start_slot), "0x00"])
        self.update_values()

        treeview = Gtk.TreeView(model=liststore)
        fontdesc = Pango.FontDescription("monospace 10")
        treeview.modify_font(fontdesc)
        renderer_slot = Gtk.CellRendererText()
        column_slot = Gtk.TreeViewColumn("Slot", renderer_slot, text=0)
        treeview.append_column(column_slot)
        renderer_value = Gtk.CellRendererText()
        column_value = Gtk.TreeViewColumn("Value", renderer_value, text=1)
        treeview.append_column(column_value)

        scrolledwindow.add(treeview)
        vbox.pack_start(scrolledwindow, True, True, 0)

        if channel_size == 3 or 9 > channel_size > 6:
            label = Gtk.Label(label="Channel Colour")
            vbox.pack_start(label, False, True, 0)

            hex_code = "ffffff"
            if channel_size == 3:
                values = self.parent.packet_last_value[self.start_slot-1:self.start_slot + 2]
                rgb = [hex(value)[2:].rjust(2, "0") for value in values]
                hex_code = "".join(rgb)
            if channel_size > 3:
                values = self.parent.packet_last_value[self.start_slot-1:self.start_slot + 3]
                rgb = [hex(int(float(value)*(float(values[0])/255.0)))[2:].rjust(2, "0") for value in values[1:4]]
                hex_code = "".join(rgb)
            color = Gdk.color_parse("#" + hex_code)
            rgba = Gdk.RGBA.from_color(color)

            area = Gtk.DrawingArea()
            area.set_size_request(24, 24)
            area.override_background_color(0, rgba)

            self.colour_area = area
            vbox.pack_start(area, False, True, 0)

            label = Gtk.Label(label="Channel Colour Hex Code")
            vbox.pack_start(label, False, True, 0)

            label = Gtk.Label(label="#" + hex_code)
            self.colour_label = label
            vbox.pack_start(label, False, True, 0)

        self.add(vbox)

        self.show_all()

        self.id = UIChannelDisplay.channel_window_index
        self.connect("destroy", self.on_close)
        UIChannelDisplay.channel_window_index += 1

    def update_values(self):
        index = 0
        for value in self.parent.packet_snapshot[self.start_slot-1:self.start_slot + self.channel_size - 1]:
            self.packet_data[index] = [str(index + self.start_slot), "0x" + hex(value)[2:].rjust(2, "0")]
            index += 1
        if self.colour_label is not None and (self.channel_size == 3 or 9 > self.channel_size > 6):
            hex_code = "ffffff"
            if self.channel_size == 3:
                values = self.parent.packet_snapshot[self.start_slot-1:self.start_slot + 2]
                rgb = [hex(value)[2:].rjust(2, "0") for value in values]
                hex_code = "".join(rgb)
            if self.channel_size > 3:
                values = self.parent.packet_snapshot[self.start_slot-1:self.start_slot + 3]
                rgb = [hex(int(float(value)*(float(values[0])/255.0)))[2:].rjust(2, "0") for value in values[1:4]]
                hex_code = "".join(rgb)
            color = Gdk.color_parse("#" + hex_code)
            rgba = Gdk.RGBA.from_color(color)

            self.colour_area.override_background_color(0, rgba)

            self.colour_label = Gtk.Label(label="#" + hex_code)
        return True

    def on_close(self, event):
        del self.parent.channel_windows[self.id]
        self.destroy()
