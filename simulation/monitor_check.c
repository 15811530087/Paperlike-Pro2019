#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "i2c-dev.h"
#include "ddcci.h"

int i2cmain(int bus, int qit);

int main(int argc, char ** argv)
{
	//i2cmain(16, 0);
	//i2cmain(17, 0);
	int i, retry, ret;
		
	/* filedescriptor and name of device */
	struct monitor mon;
	char *fn;
	
	char *datadir = NULL;
	char *pnpname = NULL; /* pnpname for -i parameter */
	
	/* what to do */
	int dump = 0;
	int ctrl = -1;
	int value = -1;
	int caps = 0;
	int save = 0;
	int force = 0;
	int verbosity = 0;
	int probe = 0;

	fn = NULL;
	
	struct monitorlist* monlist;
	struct monitorlist* current;

	if (!ddcci_init(datadir)) {
		DSPRINT(_("Unable to initialize ddcci library.\n"));
		exit(1);
	}

	monlist = ddcci_probe();

	DSPRINT(_("Detected monitors :\n"));
		
	current = monlist;
	while (current != NULL)
	{
		DSPRINT(_(" - Device: %s\n"), current->filename);
		DSPRINT(_("   DDC/CI supported: %s\n"), current->supported ? _("Yes") : _("No"));
		DSPRINT(_("   Monitor Name: %s\n"), current->name);
		DSPRINT(_("   Input type: %s\n"), current->digital ? _("Digital") : _("Analog"));
			
		if ((!fn) && (current->supported))
		{
			DSPRINT(_("  (Automatically selected)\n"));
			fn = malloc(strlen(current->filename)+1);
			strcpy(fn, current->filename);
		}

		current = current->next;
	}
		
	if (fn == NULL) {
		fprintf(stderr, _(
			"No monitor supporting DDC/CI available.\n"
			"If your graphics card need it, please check all the required kernel modules are loaded (i2c-dev, and your framebuffer driver).\n"
		    ));
		ddcci_release();
		exit(0);
	}

	ddcci_free_list(monlist);

	ddcci_release();
	return 0;
}
