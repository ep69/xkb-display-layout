/*

Copyright 2014 Stanislav Zidek

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <X11/Xlib.h>
#include <X11/XKBlib.h>

Display *display = 0;
int deviceId = XkbUseCoreKbd;
int groupCount = -1;

void Error(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

int main(void)
{
    XkbIgnoreExtension(False);

    char* displayName = strdup("");
    int eventCode;
    int errorReturn;
    int major = XkbMajorVersion;
    int minor = XkbMinorVersion;;
    int reasonReturn;
    display = XkbOpenDisplay(displayName, &eventCode, &errorReturn, &major,
                              &minor, &reasonReturn);
    switch (reasonReturn) {
    case XkbOD_BadLibraryVersion:
        Error("Bad XKB library version.");
        break;
    case XkbOD_ConnectionRefused:
        Error("Connection to X server refused.");
        break;
    case XkbOD_BadServerVersion:
        Error("Bad X11 server version.");
        break;
    case XkbOD_NonXkbServer:
        Error("XKB not present.");
        break;
    case XkbOD_Success:
        break;
    }

    XkbStateRec xkbState;
    XkbGetState(display, deviceId, &xkbState);
    int currentGroupNum = xkbState.group;
    //printf("CGN: %d\n", currentGroupNum);

    // Initialize the XKB extension.
    int baseEventCode, baseErrorCode;
    int opCode;
    XkbQueryExtension(display, &opCode, &baseEventCode, &baseErrorCode,  &major, &minor);

    XkbDescRec* kbdDescPtr = XkbAllocKeyboard();
    if (kbdDescPtr == NULL) {
        Error("Failed to get keyboard description.");
    }

    kbdDescPtr->dpy = display;
    if (deviceId != XkbUseCoreKbd) {
        kbdDescPtr->device_spec = deviceId;
    }

    XkbGetControls(display, XkbAllControlsMask, kbdDescPtr);
    XkbGetNames(display, XkbSymbolsNameMask, kbdDescPtr);
    XkbGetNames(display, XkbGroupNamesMask, kbdDescPtr);

    if (kbdDescPtr->names == NULL) {
        Error("Failed to get keyboard description.");
    }

    // Count the number of configured groups.
    const Atom* groupSource = kbdDescPtr->names->groups;
    if (kbdDescPtr->ctrls != NULL) {
        groupCount = kbdDescPtr->ctrls->num_groups;
    } else {
        groupCount = 0;
        while (groupCount < XkbNumKbdGroups &&
               groupSource[groupCount] != None) {
            groupCount++;
        }
    }

    // There is always at least one group.
    if (groupCount == 0) {
        groupCount = 1;
    }

    // Get the symbol name and parse it for layout symbols.
    Atom symNameAtom = kbdDescPtr->names->symbols;
    char *symName;
    if (symNameAtom != None) {
        symName = XGetAtomName(display, symNameAtom);
        if (!symName || !strlen(symName)) {
            Error("symName empty");
        }
    } else {
        Error("symNameAtom is None");
    }

    char *c = symName;
    while (*c && *(c++) != '+');
    //after first '+' now
    for (int i=0; i<currentGroupNum; i++) {
	    while (*c && *(c++) != '+');
    }
    if (!*c) Error("Index");
    char *result = strdup(c);
    c = result;
    while (*c && (isalpha(*c) || *c == '_')) c++;
    *c = '\0';

    XkbSelectEventDetails(display, XkbUseCoreKbd, XkbStateNotify,
                          XkbAllStateComponentsMask, XkbGroupStateMask);
 
    puts(result);
    
    free(result);
    free(displayName);
    XFree(symName);

    return 0;
}
