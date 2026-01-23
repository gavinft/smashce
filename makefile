# ----------------------------
# Makefile Options
# ----------------------------

NAME = SMASH
ICON = icon.png
DESCRIPTION = "Super Smash Bros CE"
COMPRESSED = NO

CFLAGS = -Wall -Wextra -Oz -Werror=return-type -Werror=implicit-function-declaration -Werror=incompatible-pointer-types
CXXFLAGS = -Wall -Wextra -Oz -Werror=return-type -Werror=implicit-function-declaration -Werror=incompatible-pointer-types

# ----------------------------

include $(shell cedev-config --makefile)

.PHONY: gfxclean

gfxclean:
	cd $(GFXDIR) && $(CONVIMG) -c
