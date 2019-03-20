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
	static bool can_shoot()
	{
		const c_entity local( local_player_index() );

		const auto next_primary_attack = g_ptr_memory->read_memory< float >( local.current_weapon_base() + netvars::f_next_primary_attack );
		const auto server_time = local.tickbase() * get_globalvars().interval_per_tick;

		return ( !( next_primary_attack > server_time ) );
	}
	void do_backtrack() const
	{
		if( !can_shoot() )
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

		if( ( best_simtime_ != -1 ) && ( GetAsyncKeyState( 0x1 ) & 0x8000 ) )
		{
			old_usercmd.m_iButtons |= IN_ATTACK;
			old_usercmd.m_iTickCount = time_to_ticks( best_simtime_ );
			g_ptr_memory->write_memory<usercmd_t>( ptr_old_usercmd, old_usercmd );
			g_ptr_memory->write_memory<usercmd_t>( ptr_verified_old_usercmd, old_usercmd );
		}
		send_packet( true );

	}
	void best_simtime()
	{
		if( best_target_ == -1 )
		{
			best_simtime_ = -1;
			return;
		}
		const c_entity local( local_player_index() );

		auto temp = FLT_MAX;
		const auto view_direction = angle_vector( get_viewangles() + ( local.punch_angles() * 2.f ) );
		for( auto t = 0; t < 12; ++t )
		{
			const auto temp2 = distance_point_to_line( backtrack_positions_[ best_target_ ][ t ].hitboxpos, local.eye_postition(), view_direction );
			if( temp > temp2 && backtrack_positions_[ best_target_ ][ t ].simtime > local.simulation_time() - 1 )
			{
				temp = temp2;
				best_simtime_ = backtrack_positions_[ best_target_ ][ t ].simtime;
			}
		}
		if( max_backtrack_ms_ > 0 && !is_valid_tick( time_to_ticks( best_simtime_ ) ) )
			best_simtime_ = -1;
	}
	void update()
	{
		best_target_ = -1;

		const auto local_index = local_player_index();
		const c_entity local( local_index );
		auto best_fov = FLT_MAX;
		if( local.health() < 1 )
			return;

		for( auto i = 0; i < get_globalvars().maxClients; i++ )
		{
			const c_entity entity( i );

			if( i == local_index )
				continue;

			if( entity.dormant() )
				continue;

			if( entity.team() == local.team() )
				continue;

			if( entity.health() > 0 )
			{
				const auto simtime = entity.simulation_time();
				const auto head_position = entity.bone_position( 8 );

				backtrack_positions_[ i ][ get_globalvars().tickcount % 13 ] = backtrack_data_t{ simtime, head_position };
				const auto view_direction = angle_vector( get_viewangles() + ( local.punch_angles() * 2.f ) );
				const auto fov_distance = distance_point_to_line( head_position, local.eye_postition(), view_direction );
				if( best_fov > fov_distance )
				{
					best_fov = fov_distance;
					best_target_ = i;
				}
			}
		}
	}
private:
	static netchannel_t get_netchannel()
	{
		return g_ptr_memory->read_memory<netchannel_t>( g_ptr_memory->read_memory<ptrdiff_t>( offsets::dw_clientstate + netvars::dw_netchannel ) );
	}
	bool is_valid_tick( const int tick ) const
	{
		const auto gvars = get_globalvars();
		const auto delta = gvars.tickcount - tick;
		const auto delta_time = delta * gvars.interval_per_tick;
		const auto max = static_cast< float >( static_cast < float >( max_backtrack_ms_ ) / static_cast < float >( 1000 ) );
		return ( fabs( delta_time ) <= max );

	}
	static double get_nextcmdtime()
	{
		return g_ptr_memory->read_memory<double>( offsets::dw_clientstate + netvars::dw_next_cmd );
	}
	static globalvars_t get_globalvars()
	{
		return g_ptr_memory->read_memory<globalvars_t>( engine_module->get_image_base() + offsets::dw_globalvars );
	}
	static void set_tick_count( const int tick )
	{
		g_ptr_memory->write_memory<int>( engine_module->get_image_base() + offsets::dw_globalvars + 0x1C, tick );
	}
	static Vector get_viewangles()
	{
		return g_ptr_memory->read_memory<Vector>( offsets::dw_clientstate + netvars::vec_view_angles );
	}
	static int time_to_ticks( float time )
	{
		return static_cast< int >( static_cast< float >( 0.5f ) + static_cast< float >( time ) / static_cast< float >( get_globalvars().interval_per_tick ) );
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

	backtrack_data_t backtrack_positions_[ 64 ][ 12 ] = { 0.0f, Vector(0.0f, 0.0f, 0.0f) };
	int best_target_ = -1;
	float best_simtime_ = -1;
	int max_backtrack_ms_ = 200; // how many millisecond we want our backtrack to be max
};

extern std::unique_ptr<c_backtrack> g_ptr_backtrack;