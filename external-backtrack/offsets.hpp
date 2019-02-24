#pragma once

namespace offsets
{
	ptrdiff_t dw_clientstate = 0x58BCFC;
	ptrdiff_t dw_entitylist = 0x4CDB00C;
	ptrdiff_t dw_globalvars = 0x58BA00;
	ptrdiff_t dw_sendpacket = 0xD230A;
	ptrdiff_t dw_last_outgoing_command = 0x4D24;
	ptrdiff_t dw_input = 0x5125DA0;
}

namespace netvars
{
	ptrdiff_t i_sigonstate = 0x108;
	ptrdiff_t i_local = 0x180;
	ptrdiff_t i_health = 0x100;
	ptrdiff_t i_team = 0xF4;

	ptrdiff_t b_dormant = 0xED;

	ptrdiff_t f_simulation_time = 0x268;

	ptrdiff_t dw_bonematrix = 0x26A8;

	ptrdiff_t vec_aim_punch_angles = 0x302C;
	ptrdiff_t vec_origin = 0x138;
	ptrdiff_t vec_view_offset = 0x108;
	ptrdiff_t vec_view_angles = 0x4D88;
}