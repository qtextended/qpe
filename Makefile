default:  dynamic

LIBS=library

COMPONENTS=inputmethods/keyboard \
	inputmethods/pickboard \
	inputmethods/handwriting \
	inputmethods/unikeyboard \
	taskbar/batteryapplet \
	taskbar/volumeapplet \
	taskbar/clockapplet \
	cardmon

APPS=$(LOCALAPPS) addressbook calculator clock datebook \
    filebrowser helpbrowser minesweep mpegplayer \
    settings/light-and-power settings/language \
    settings/rotation settings/appearance settings/systemtime \
    solitaire spreadsheet tetrix textedit \
    todo tux wordgame embeddedkonsole taskbar sysinfo \
    parashoot snake citytime showimg netsetup \
    qipkg qppp mindbreaker go qasteroids qcop

single mpegplayer: libmpeg

libmpeg:
	$(MAKE) -C mpegplayer/libmpeg3

dynamic: $(APPS)

$(APPS): $(LIBS) $(COMPONENTS)

$(LIBS) $(COMPONENTS) $(APPS) single:
	$(MAKE) -C $@ -f Makefile

showcomponents:
	@echo $(LIBS) $(APPS) $(COMPONENTS) single

clean:
	$(MAKE) -C mpegplayer/libmpeg3 $@
	$(MAKE) -C single -f Makefile $@
	for dir in $(APPS) $(LIBS) $(COMPONENTS); do $(MAKE) -C $$dir -f Makefile $@ || exit 1; done


.PHONY: default dynamic $(LIBS) $(APPS) $(COMPONENTS) libmpeg single showcomponents clean
