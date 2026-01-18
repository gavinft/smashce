# ----------------------------
# Makefile Options
# ----------------------------

NAME = SMASH
ICON = icon.png
DESCRIPTION = "Super Smash Bros CE"
COMPRESSED = NO

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)
