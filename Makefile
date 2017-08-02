CFLAG=-Iinclude/ -I/usr/include/ $(shell pkg-config --cflags --libs gtk+-2.0)
LDFLAG=-ldl -lpthread -lX11 -lm -lXrandr
DEST=obj
OBJS=$(DEST)/main.o $(DEST)/create_main_window.o $(DEST)/hotplug_monitor.o\
	$(DEST)/kbd_event.o $(DEST)/mouse_event.o $(DEST)/mouse_wheel_click.o\
	$(DEST)/ddcci.o $(DEST)/amd_adl.o $(DEST)/thread.o\
	$(DEST)/manage_window_show_hide.o $(DEST)/create_setting_window.o\
	$(DEST)/create_help_window.o $(DEST)/create_about_window.o

RESOBJS=$(DEST)/resolution_change.o

CC=gcc
SIM=simulation
SIMFLAG=-DSIMULATION

default:PaperlikePro ResChange buildpackage

.PHONY:buildpackage
buildpackage:
	cp PaperlikePro ResChange DS.ico package/usr/local/sbin/
	dpkg -b package .

.PHONY:PaperlikePro
PaperlikePro:$(OBJS)
	gcc -o $@ $(OBJS) $(CFLAG) $(LDFLAG)
	strip $@

.PHONY:ResChange
ResChange:$(RESOBJS)
	gcc -o $@ $(RESOBJS) $(CFLAG) $(LDFLAG)
	strip $@

$(DEST)/ddcci.o : DSddc/ddcci.c
	$(CC) -c -o $@ $< $(CFLAG) $(GTKINC)

$(DEST)/amd_adl.o : DSddc/amd_adl.c
	$(CC) -c -o $@ $< $(CFLAG)

$(DEST)/hotplug_monitor.o : hotplug/hotplug_monitor.c
	$(CC) -c -o $@ $< $(CFLAG)

$(DEST)/kbd_event.o : hotplug/kbd_event.c
	$(CC) -c -o $@ $< $(CFLAG)

$(DEST)/mouse_event.o : hotplug/mouse_event.c
	$(CC) -c -o $@ $< $(CFLAG)

$(DEST)/mouse_wheel_click.o : hotplug/mouse_wheel_click.c
	$(CC) -c -o $@ $< $(CFLAG)

$(DEST)/resolution_change.o : hotplug/resolution_change.c
	$(CC) -c -o $@ $< $(CFLAG) $(SIMFLAG)

$(DEST)/create_main_window.o : ui/create_main_window.c
	$(CC) -c -o $@ $< $(CFLAG)

$(DEST)/main.o : ui/main.c
	$(CC) -c -o $@ $< $(CFLAG)

$(DEST)/thread.o : ui/thread.c
	$(CC) -c -o $@ $< $(CFLAG)

$(DEST)/create_setting_window.o : ui/create_setting_window.c
	$(CC) -c -o $@ $< $(CFLAG)

$(DEST)/create_help_window.o : ui/create_help_window.c
	$(CC) -c -o $@ $< $(CFLAG)

$(DEST)/create_about_window.o : ui/create_about_window.c
	$(CC) -c -o $@ $< $(CFLAG)

$(DEST)/manage_window_show_hide.o : ui/manage_window_show_hide.c
	$(CC) -c -o $@ $< $(CFLAG)

.PHONY:simulation
simulation:
	$(CC) -o $(SIM)/hotplug_test $(SIM)/hotplug_monitor.c $(SIMFLAG)
	$(CC) -o $(SIM)/monitor_probe $(SIM)/monitor_check.c DSddc/ddcci.c DSddc/amd_adl.c $(CFLAG) $(LDFLAG) $(SIMFLAG)
	$(CC) -o $(SIM)/keyboard_event hotplug/kbd_event.c $(SIMFLAG)
	$(CC) -o $(SIM)/mouse_event hotplug/mouse_event.c $(SIMFLAG) $(LDFLAG)
	$(CC) -o $(SIM)/mouse_wheel_event hotplug/mouse_wheel_click.c $(SIMFLAG) $(LDFLAG)

.PHONY:clean
clean:
	-rm -rf $(DEST)/*.o 2>&1 > /dev/null
	-rm -rf PaperlikePro 2>&1 > /dev/null
	-rm -rf ResChange 2>&1 > /dev/null
	-rm -rf paper*.deb 2>&1 > /dev/null
	-rm -rf package/usr/local/sbin/* 2>&1 > /dev/null
	-rm -rf $(SIM)/hotplug_test 2>&1 > /dev/null
	-rm -rf $(SIM)/monitor_probe 2>&1 > /dev/null
	-rm -rf $(SIM)/keyboard_event 2>&1 > /dev/null
	-rm -rf $(SIM)/mouse_event 2>&1 > /dev/null
	-rm -rf $(SIM)/mouse_wheel_event 2>&1 > /dev/null


