#include "config.h"
#include "utils.h"

Hotkey::Hotkey(unsigned int key)
	: virtualKey(key)
	, label(utils::VirtualKeyToString(key))
{
}
