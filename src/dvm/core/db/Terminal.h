// Licensed under the GNU General Public License, Version 3.
#pragma once

namespace dev
{
namespace con
{

#if defined(_WIN32)

#define DVMReset ""       // Text Reset

#define DVMReset ""       // Text Reset

	// Regular Colors
#define DVMBlack	""    // Black
#define DVMCoal		""    // Black
#define DVMGray		""    // White
#define DVMWhite	""    // White
#define DVMMaroon	""    // Red
#define DVMRed		""    // Red
#define DVMGreen	""    // Green
#define DVMLime		""    // Green
#define DVMOrange	""    // Yellow
#define DVMYellow	""    // Yellow
#define DVMNavy		""    // Blue
#define DVMBlue		""    // Blue
#define DVMViolet	""    // Purple
#define DVMPurple	""    // Purple
#define DVMTeal		""    // Cyan
#define DVMCyan		""    // Cyan

#define DVMBlackBold	""      // Black
#define DVMCoalBold		""      // Black
#define DVMGrayBold		""      // White
#define DVMWhiteBold	""      // White
#define DVMMaroonBold	""      // Red
#define DVMRedBold		""      // Red
#define DVMGreenBold	""      // Green
#define DVMLimeBold		""      // Green
#define DVMOrangeBold	""      // Yellow
#define DVMYellowBold	""      // Yellow
#define DVMNavyBold		""      // Blue
#define DVMBlueBold		""      // Blue
#define DVMVioletBold	""      // Purple
#define DVMPurpleBold	""      // Purple
#define DVMTealBold		""      // Cyan
#define DVMCyanBold		""      // Cyan

	// Background
#define DVMOnBlack		""       // Black
#define DVMOnCoal		""		 // Black
#define DVMOnGray		""       // White
#define DVMOnWhite		""		 // White
#define DVMOnMaroon		""       // Red
#define DVMOnRed		""       // Red
#define DVMOnGreen		""       // Green
#define DVMOnLime		""		 // Green
#define DVMOnOrange		""       // Yellow
#define DVMOnYellow		""		 // Yellow
#define DVMOnNavy		""       // Blue
#define DVMOnBlue		""		 // Blue
#define DVMOnViolet		""       // Purple
#define DVMOnPurple		""		 // Purple
#define DVMOnTeal		""       // Cyan
#define DVMOnCyan		""		 // Cyan

	// Underline
#define DVMBlackUnder	""       // Black
#define DVMGrayUnder	""       // White
#define DVMMaroonUnder	""       // Red
#define DVMGreenUnder	""       // Green
#define DVMOrangeUnder	""       // Yellow
#define DVMNavyUnder	""       // Blue
#define DVMVioletUnder	""       // Purple
#define DVMTealUnder	""       // Cyan

#else

#define DVMReset "\x1b[0m"       // Text Reset

// Regular Colors
#define DVMBlack "\x1b[30m"        // Black
#define DVMCoal "\x1b[90m"       // Black
#define DVMGray "\x1b[37m"        // White
#define DVMWhite "\x1b[97m"       // White
#define DVMMaroon "\x1b[31m"          // Red
#define DVMRed "\x1b[91m"         // Red
#define DVMGreen "\x1b[32m"        // Green
#define DVMLime "\x1b[92m"       // Green
#define DVMOrange "\x1b[33m"       // Yellow
#define DVMYellow "\x1b[93m"      // Yellow
#define DVMNavy "\x1b[34m"         // Blue
#define DVMBlue "\x1b[94m"        // Blue
#define DVMViolet "\x1b[35m"       // Purple
#define DVMPurple "\x1b[95m"      // Purple
#define DVMTeal "\x1b[36m"         // Cyan
#define DVMCyan "\x1b[96m"        // Cyan

#define DVMBlackBold "\x1b[1;30m"       // Black
#define DVMCoalBold "\x1b[1;90m"      // Black
#define DVMGrayBold "\x1b[1;37m"       // White
#define DVMWhiteBold "\x1b[1;97m"      // White
#define DVMMaroonBold "\x1b[1;31m"         // Red
#define DVMRedBold "\x1b[1;91m"        // Red
#define DVMGreenBold "\x1b[1;32m"       // Green
#define DVMLimeBold "\x1b[1;92m"      // Green
#define DVMOrangeBold "\x1b[1;33m"      // Yellow
#define DVMYellowBold "\x1b[1;93m"     // Yellow
#define DVMNavyBold "\x1b[1;34m"        // Blue
#define DVMBlueBold "\x1b[1;94m"       // Blue
#define DVMVioletBold "\x1b[1;35m"      // Purple
#define DVMPurpleBold "\x1b[1;95m"     // Purple
#define DVMTealBold "\x1b[1;36m"        // Cyan
#define DVMCyanBold "\x1b[1;96m"       // Cyan

// Background
#define DVMOnBlack "\x1b[40m"       // Black
#define DVMOnCoal "\x1b[100m"   // Black
#define DVMOnGray "\x1b[47m"       // White
#define DVMOnWhite "\x1b[107m"   // White
#define DVMOnMaroon "\x1b[41m"         // Red
#define DVMOnRed "\x1b[101m"     // Red
#define DVMOnGreen "\x1b[42m"       // Green
#define DVMOnLime "\x1b[102m"   // Green
#define DVMOnOrange "\x1b[43m"      // Yellow
#define DVMOnYellow "\x1b[103m"  // Yellow
#define DVMOnNavy "\x1b[44m"        // Blue
#define DVMOnBlue "\x1b[104m"    // Blue
#define DVMOnViolet "\x1b[45m"      // Purple
#define DVMOnPurple "\x1b[105m"  // Purple
#define DVMOnTeal "\x1b[46m"        // Cyan
#define DVMOnCyan "\x1b[106m"    // Cyan

// Underline
#define DVMBlackUnder "\x1b[4;30m"       // Black
#define DVMGrayUnder "\x1b[4;37m"       // White
#define DVMMaroonUnder "\x1b[4;31m"      // Red
#define DVMGreenUnder "\x1b[4;32m"       // Green
#define DVMOrangeUnder "\x1b[4;33m"      // Yellow
#define DVMNavyUnder "\x1b[4;34m"        // Blue
#define DVMVioletUnder "\x1b[4;35m"      // Purple
#define DVMTealUnder "\x1b[4;36m"        // Cyan

#endif

}

}
