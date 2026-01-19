# ----------------------------
# Makefile Options
# ----------------------------

NAME = SMASH
ICON = icon.png
DESCRIPTION = "Super Smash Bros CE"
COMPRESSED = NO

CFLAGS = -Wall -Wextra -Oz -Werror=return-type -Werror=implicit-function-declaration
CXXFLAGS = -Wall -Wextra -Oz -Werror=return-type -Werror=implicit-function-declaration

# ----------------------------

include $(shell cedev-config --makefile)
