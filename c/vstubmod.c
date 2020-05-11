#include "vstub.h"

unsigned	n_mods_bound;
vstubmod_t	*mods_bound[MAX_STUBS];

extern vstubmod_t	vstubmod_hid_mouse;
extern vstubmod_t	vstubmod_hid_keyboard;
extern vstubmod_t	vstubmod_cdc_acm;
extern vstubmod_t	vstubmod_bth;
extern vstubmod_t	vstubmod_cp210x;
extern vstubmod_t	vstubmod_avrmkii;
extern vstubmod_t	vstubmod_arduino;

static vstubmod_t	*mods_all[] = {
	&vstubmod_hid_mouse,
	&vstubmod_hid_keyboard,
	&vstubmod_cdc_acm,
	&vstubmod_bth,
	&vstubmod_cp210x,
	&vstubmod_avrmkii,
	&vstubmod_arduino
};

vstubmod_t *
find_vstubmod(unsigned devno)
{
	if (devno == 0)
		return NULL;
	if (n_mods_bound < devno)
		return NULL;
	return mods_bound[devno - 1];
}

static void
add_bound_vstubmod(vstubmod_t *mod)
{
	mods_bound[n_mods_bound] = mod;
	n_mods_bound++;
}

static void
bind_all_modules(void)
{
	int	i;

	for (i = 0; i < sizeof(mods_all) / sizeof(vstubmod_t *); i++) {
		add_bound_vstubmod(mods_all[i]);
	}
}

static vstubmod_t *
find_vstubmod_bycode(const char *code)
{
	int	i;

	for (i = 0; i < sizeof(mods_all) / sizeof(vstubmod_t *); i++) {
		if (strcmp(code, mods_all[i]->code) == 0)
			return mods_all[i];
	}
	return NULL;
}

BOOL
setup_vstubmods(int argc, char *argv[])
{
	int	i;

	if (argc == 0) {
		bind_all_modules();
		return TRUE;
	}
	if (argc > MAX_STUBS) {
		error("too many stubs to bind");
		return FALSE;
	}

	for (i = 0; i < argc; i++) {
		vstubmod_t	*mod = find_vstubmod_bycode(argv[i]);

		if (mod == NULL) {
			error("no matching module: %s", argv[i]);
			return FALSE;
		}
		add_bound_vstubmod(mod);
	}

	return TRUE;
}
