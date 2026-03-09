#include "config.h"
#include "lib/utils/utils.h"

Hotkey::Hotkey(unsigned int key)
	: virtualKey(key)
	, label(utils::VirtualKeyToString(key))
{
}
