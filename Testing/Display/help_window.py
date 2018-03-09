#!/usr/bin/env python3

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk

TITLE_MARKUP = """<big>EMPR PC Display</big>
<i>By Group 4: Kia Alderson, Maxime Franchot, Jacob Allen, and Calum Ferguson</i>"""

HELP_MARKUP = """<big>About</big>
The EMPR PC Display can be used to display information about a DMX line \
connected to an EMPR Monitor Board via a computer.

<big>Help</big>
<b>Configuring the Monitor</b>
To start capturing packets, first make sure the Monitor is in "DISPLAY MODE" \
this can be done by pressing the "5" key on the keypad when on the main menu \
of the Monitor. The Monitor should display a message saying it is now in \
"DISPLAY MODE".

<b>Receiving Packets</b>
The Display should now be receiving packets from the Monitor. You can confirm \
this by looking at the "Pckets Received Count" which should be increasing as \
packets are sent via the DMX line. (<b>NB:</b> This count will only \
increase if the Monitor is actually receiving DMX data, as such make sure that \
the DMX line is connect to the monitor and data is being sent on the DMX line.)

<b>Capturing Packets</b>
You should now be able to capture individual packets by clicking on the \
"Capture Packet" button, or continuously capture packets by clicking on the \
"Start Continuous Capture" button. Captured packets should appear in the in \
table to the right of the main Display window. Each packet displayed will \
increment the "Packets Displayed Count" by 1.

<b>Inspecting Whole Packets</b>
The captured packets area to the right of the main Display window shows the \
read value of all slots in the previously captured packet. From this table you \
can scroll through the 512 DMX slots and see their value and index within the \
DMX packet.

<b>Monitoring Channels</b>
To monitor a particular channel, first select the type of channel from the \
"Channel Monitor" drop-down menu. Then highlight the first slot of the channel \
you wish to monitor. (For instance if you highlight slot 12, you will monitor \
the channel starting with 12, so slots 12, 13, 14 etc.) Then to open a new \
channel monitor window click the "New Channel Monitor" button. This should \
open a new window which will display the state of the selected channel kept in \
sync with the current state of the main monitor window.
"""

class UIHelp(Gtk.Window):
    channel_window_index = 0

    def __init__(self, parent):
        Gtk.Window.__init__(self, title="EMPR Display - Help Window")
        self.parent = parent
        self.set_default_size(400,400)
        self.set_border_width(10)

        vbox = Gtk.Box(orientation=Gtk.Orientation.VERTICAL, spacing=10)
        vbox.set_homogeneous(False)

        label = Gtk.Label()
        label.set_line_wrap(True)
        label.set_markup(TITLE_MARKUP)
        label.set_justify(Gtk.Justification.CENTER)
        vbox.pack_start(label, False, True, 0)

        label = Gtk.Label()
        label.set_line_wrap(True)
        label.set_markup(HELP_MARKUP)
        label.set_justify(Gtk.Justification.LEFT)
        vbox.pack_start(label, False, True, 0)

        scrolledwindow = Gtk.ScrolledWindow()
        scrolledwindow.set_policy(Gtk.PolicyType.NEVER, Gtk.PolicyType.AUTOMATIC)

        scrolledwindow.add(vbox)

        self.add(scrolledwindow)

        self.show_all()

    def on_close(self, event):
        self.destroy()
