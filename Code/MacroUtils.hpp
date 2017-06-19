#pragma once
#define CONCAT_IMPL( x, y )		x##y
#define MACRO_CONCAT( x, y )	CONCAT_IMPL( x, y )