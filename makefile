# ----------------------------
# Makefile Options
# ----------------------------

NAME = SMASH
ICON = icon.png
DESCRIPTION = "Super Smash Bros CE"
COMPRESSED = NO

CFLAGS = -Wall -Wextra -Oz -Werror=return-type
CXXFLAGS = -Wall -Wextra -Oz -Werror=return-type

# ----------------------------

include $(shell cedev-config --makefile)
