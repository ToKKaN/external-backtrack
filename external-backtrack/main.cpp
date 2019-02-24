#include "stdafx.hpp"
#include <thread>
#include <process.h> 

std::unique_ptr<c_memory> g_ptr_memory				= std::make_unique<c_memory>();
std::unique_ptr<c_backtrack> g_ptr_backtrack		= std::make_unique<c_backtrack>();
c_module* engine_module = nullptr;
c_module* client_module = nullptr;
bool end = false;

unsigned int __stdcall backtrack()
{
	for( ;; )
	{
		if( end )
			break;

		if( g_ptr_backtrack->is_ingame() && GetForegroundWindow() == g_ptr_memory->get_window() )
		{
			if( GetAsyncKeyState(0x1) & 0x8000 )
				g_ptr_backtrack->do_backtrack();
		}
		std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
	}

	return 0;
}
unsigned int __stdcall update_data()
{
	for( ;; )
	{
		if( end )
			break;

		if( g_ptr_backtrack->is_ingame() )
		{
			g_ptr_backtrack->update();
		}
		std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
	}

	return 0;
}
unsigned int __stdcall best_simtime()
{
	for( ;; )
	{
		if( end )
			break;

		if( g_ptr_backtrack->is_ingame() )
		{
			g_ptr_backtrack->best_simtime();
		}
		std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
	}

	return 0;
}
unsigned int __stdcall hotkey()
{
	for( ;; )
	{
		if( end )
			break;

		if( GetAsyncKeyState( VK_F6 ) & 1 )
			end = true;

		std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
	}

	return 0;
}
int main()
{
	printf_s( "\t\t\t\t\t\texternal-backtrack POC\n\n" );

	printf_s( "Waiting for Counter-Strike: Global Offensive\n" );
	HWND game_window = nullptr;
	while( !game_window )
	{
		game_window = FindWindowA( nullptr, "Counter-Strike: Global Offensive" );
		std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
	}
	std::this_thread::sleep_for( std::chrono::milliseconds( 1000 ) );
	while( !g_ptr_memory->attach_process("csgo.exe") )
	{
		std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
	}
	printf_s( "Counter-Strike: Global Offensive found\n" );
	
	while( !engine_module || !client_module )
	{
		engine_module = g_ptr_memory->get_module( "engine.dll" );
		client_module = g_ptr_memory->get_module( "client_panorama.dll" );

		std::this_thread::sleep_for( std::chrono::milliseconds( 200 ) );
	}

	g_ptr_memory->set_process_window( game_window );

	if( engine_module && client_module )
		printf_s( "engine and client module found\n" );
	else
	{
		printf_s( "can not find engine and client module... exiting now\n" );
		return -1;
	}

	offsets::dw_clientstate = g_ptr_memory->read_memory<ptrdiff_t>( engine_module->get_image_base() + offsets::dw_clientstate );

	std::thread t1( backtrack );
	std::thread t2( update_data );
	std::thread t3( best_simtime );
	std::thread t4( hotkey );

	t1.join();
	t2.join();
	t3.join();
	t4.join();

	g_ptr_backtrack->send_packet( true );

	g_ptr_memory->detach_process();
	delete engine_module;
	delete client_module;

	return 0;
}
