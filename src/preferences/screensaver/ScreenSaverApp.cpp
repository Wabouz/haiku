/*
 * Copyright 2003-2013 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Jérôme Duval, jerome.duval@free.fr
 *		Michael Phipps
 */


#include <Application.h>
#include <Entry.h>
#include <Path.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "ScreenSaverWindow.h"


class ScreenSaverApp : public BApplication {
public:
	ScreenSaverApp();
	virtual void RefsReceived(BMessage* message);

private:
	BWindow* fScreenSaverWindow;
};


//	#pragma mark - ScreenSaverApp


ScreenSaverApp::ScreenSaverApp()
	:
	BApplication("application/x-vnd.Haiku-ScreenSaver")
{
	fScreenSaverWindow = new ScreenSaverWindow();
	fScreenSaverWindow->Show();
}


void
ScreenSaverApp::RefsReceived(BMessage* message)
{
	entry_ref ref;
	if (message->FindRef("refs", &ref) != B_OK)
		return;

	// Install the screen saver by copying it to the add-ons directory
	// TODO: the translator have a similar mechanism - this could be cleaned
	//       up and have one nicely working solution
	// TODO: should test if the dropped ref is really a screen saver!
	// TODO: you can receive more than one ref at a time...

	BEntry entry;
	entry.SetTo(&ref, true);
	if (entry.InitCheck() != B_OK)
		return;

	BPath path;
	entry.GetPath(&path);

	// TODO: find_directory() anyone??
	char temp[B_PATH_NAME_LENGTH * 2];
	sprintf(temp, "cp %s '/boot/home/config/add-ons/Screen Savers/'\n",
		path.Path());
	system(temp);

	fScreenSaverWindow->PostMessage(kMsgUpdateList);
}


//	#pragma mark - main()


int
main()
{
	ScreenSaverApp app;
	app.Run();
	return 0;
}
