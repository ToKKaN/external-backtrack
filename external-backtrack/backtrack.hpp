#pragma once
#define PI 3.14159265358979323846f
#include "vector.hpp"
#include "offsets.hpp"
#include <thread>

class c_backtrack
{
public:
	static bool is_ingame()
	{
		return ( g_ptr_memory->read_memory<int>( offsets::dw_clientstate + netvars::i_sigonstate ) == 6 );
	}
	static void send_packet( const bool status )
	{
		const BYTE val = status ? 1 : 0;
		g_ptr_memory->write_memory_protected<BYTE>( engine_module->get_image_base() + offsets::dw_sendpacket, val );
	}
	static int local_player_index()
	{
		return g_ptr_memory->read_memory<int>( offsets::dw_clientstate + netvars::i_local );
	}
	void do_backtrack()
	{
		const auto ptr_local = std::make_unique<c_entity>( local_player_index() );
		if( ptr_local->health() < 1 )
			return;

		const auto current_sequence_number = g_ptr_memory->read_memory<int>( offsets::dw_clientstate + offsets::dw_last_outgoing_command ) + 2;
		send_packet( false );
		const auto input = g_ptr_memory->read_memory<input_t>( client_module->get_image_base() + offsets::dw_input );

		const auto ptr_usercmd = input.m_pCommands + ( current_sequence_number % 150 ) * sizeof( usercmd_t );

		const auto ptr_old_usercmd = input.m_pCommands + ( ( current_sequence_number - 1 ) % 150 ) * sizeof( usercmd_t );
		const auto ptr_verified_old_usercmd = input.m_pVerifiedCommands + ( ( current_sequence_number - 1 ) % 150 ) * sizeof( verified_usercmd_t );

		while( g_ptr_memory->read_memory<int32_t>( ptr_usercmd + 0x4 ) < current_sequence_number )
		{
			std::this_thread::yield();
		}

		auto old_usercmd = g_ptr_memory->read_memory<usercmd_t>( ptr_old_usercmd );

		if( best_target_ != -1 && best_simtime_ != -1 )
		{
			old_usercmd.m_iButtons = IN_ATTACK;
			old_usercmd.m_iTickCount = time_to_ticks( best_simtime_ );
			g_ptr_memory->write_memory<usercmd_t>( ptr_old_usercmd, old_usercmd );
			g_ptr_memory->write_memory<usercmd_t>( ptr_verified_old_usercmd, old_usercmd );
		}
		send_packet( true );
	}
	void best_simtime()
	{
		const auto ptr_local = std::make_unique<c_entity>( local_player_index() );
		if( ptr_local->health() < 1 )
			return;

		auto temp = FLT_MAX;
		const auto view_direction = angle_vector( get_viewangles() + ( ptr_local->punch_angles() * 2.f ) );
		for( auto t = 0; t < 12; ++t )
		{
			const auto temp2 = distance_point_to_line( backtrack_positions[ best_target_ ][ t ].hitboxpos, ptr_local->eye_postition(), view_direction );
			if( temp > temp2 && backtrack_positions[ best_target_ ][ t ].simtime > ptr_local->simulation_time() - 1 )
			{
				temp = temp2;
				best_simtime_ = backtrack_positions[ best_target_ ][ t ].simtime;
			}
		}
	}
	void update()
	{
		auto local_index = local_player_index();
		const auto ptr_local = std::make_unique<c_entity>( local_index );
		auto best_fov = FLT_MAX;
		if( ptr_local->health() < 1 )
			return;

		for( auto i = 0; i < get_globalvars().maxClients; i++ )
		{
			const auto ptr_entity = std::make_unique<c_entity>( i );

			if( i == local_index )
				continue;

			if( ptr_entity->dormant() )
				continue;

			if( ptr_entity->team() == ptr_local->team() )
				continue;

			if( ptr_entity->health() > 0 )
			{

				const auto current_sequence_number = g_ptr_memory->read_memory<int>( offsets::dw_clientstate + offsets::dw_last_outgoing_command ) + 2;

				const auto input = g_ptr_memory->read_memory<input_t>( client_module->get_image_base() + offsets::dw_input );

				const auto ptr_usercmd = input.m_pCommands + ( current_sequence_number % 150 ) * sizeof( usercmd_t );
				auto cmd = g_ptr_memory->read_memory<usercmd_t>( ptr_usercmd );

				const auto simtime = ptr_entity->simulation_time();
				const auto head_position = ptr_entity->get_bone_position( 8 );

				backtrack_positions[ i ][ cmd.m_iCmdNumber % 13 ] = backtrack_data_t{ simtime, head_position };
				const auto view_direction = angle_vector( cmd.m_vecViewAngles + ( ptr_local->punch_angles() * 2.f ) );
				const auto fov_distance = distance_point_to_line( head_position, ptr_local->eye_postition(), view_direction );

				if( best_fov > fov_distance )
				{
					best_fov = fov_distance;
					best_target_ = i;
				}
			}
		}
	}
private:
	static globalvars_t get_globalvars()
	{
		return g_ptr_memory->read_memory<globalvars_t>( engine_module->get_image_base() + offsets::dw_globalvars );
	}
	static void set_globalvars( const globalvars_t val )
	{
		g_ptr_memory->write_memory<globalvars_t>( engine_module->get_image_base() + offsets::dw_globalvars, val );
	}
	static Vector get_viewangles()
	{
		return g_ptr_memory->read_memory<Vector>( offsets::dw_clientstate + netvars::vec_view_angles );
	}
	static int time_to_ticks( float time )
	{
		return static_cast< int >( 0.5f + static_cast< float >( time ) / get_globalvars().interval_per_tick );
	}
	static Vector angle_vector( const Vector in )
	{
		const auto sin_y = sin( in.y / 180.f * static_cast< float >( PI ) );
		const auto sin_x = sin( in.x / 180.f * static_cast< float >( PI ) );

		const auto cos_y = cos( in.y / 180.f * static_cast< float >( PI ) );
		const auto cos_x = cos( in.x / 180.f* static_cast< float >( PI ) );

		return Vector( cos_x*cos_y, cos_x*sin_y, -sin_x );
	}
	static float distance_point_to_line( Vector point, Vector line, Vector direction )
	{
		auto point_direction = point - line;

		const auto temp = point_direction.Dot( direction ) / ( direction.x*direction.x + direction.y*direction.y + direction.z*direction.z );
		if( temp < 0.000001f )
			return FLT_MAX;

		const auto perpen_point = line + ( direction * temp );

		return ( point - perpen_point ).Length();
	}

	backtrack_data_t backtrack_positions[ 64 ][ 12 ];
	int best_target_ = -1;
	float best_simtime_ = -1;
};

extern std::unique_ptr<c_backtrack> g_ptr_backtrack;