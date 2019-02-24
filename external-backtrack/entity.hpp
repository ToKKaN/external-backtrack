#pragma once
#include "offsets.hpp"

class c_entity
{
public:
	c_entity( const int i, const ptrdiff_t base = NULL )
	{
		if( base == NULL )
		{
			baseaddress_ = g_ptr_memory->read_memory<ptrdiff_t>( client_module->get_image_base() + offsets::dw_entitylist + ( i * 0x10 ) );
			index_ = i;
		}
		else
		{
			baseaddress_ = base;
			index_ = 0;
		}
	}
	int health() const
	{
		return g_ptr_memory->read_memory<int>( baseaddress_ + netvars::i_health );
	}
	bool dormant() const 
	{
		return g_ptr_memory->read_memory<bool>( baseaddress_ + netvars::b_dormant );
	}
	int team() const 
	{
		return g_ptr_memory->read_memory<int>( baseaddress_ + netvars::i_team );
	}
	float simulation_time() const 
	{
		return g_ptr_memory->read_memory<float>( baseaddress_ + netvars::f_simulation_time );
	}
	Vector get_bone_position( const int bone ) const
	{
		Vector out;
		const auto temp = g_ptr_memory->read_memory<ptrdiff_t>( baseaddress_ + netvars::dw_bonematrix );
		out.x = g_ptr_memory->read_memory<float>( temp + 0x30 * bone + 0xC );
		out.y = g_ptr_memory->read_memory<float>( temp + 0x30 * bone + 0x1C );
		out.z = g_ptr_memory->read_memory<float>( temp + 0x30 * bone + 0x2C );

		return out;
	}
	Vector punch_angles() const
	{
		return g_ptr_memory->read_memory<Vector>( baseaddress_ + netvars::vec_aim_punch_angles );
	}
	Vector origin() const
	{
		return g_ptr_memory->read_memory<Vector>( baseaddress_ + netvars::vec_origin );
	}
	Vector eye_postition() const
	{
		Vector eye = g_ptr_memory->read_memory<Vector>( baseaddress_ + netvars::vec_view_offset );
		eye += origin();

		return eye;
	}
private:
	ptrdiff_t baseaddress_;
	int index_;
};
