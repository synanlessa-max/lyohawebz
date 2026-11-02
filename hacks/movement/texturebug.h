#pragma once

#include "texturebug_wallstrafe.h"

class c_user_cmd;

/* texture bug — auto seam ride / bounce / run-up align, ported from the def ( botox ) sources.
   sound on detection intentionally NOT ported ( owner request ). */
namespace n_texturebug
{
	struct impl_t {
	public:
		void on_create_move( c_user_cmd* cmd );
		// grounded half: aims run at face, holds jump until arc meets target height
		// texture_bug never sees a grounded tick
		void tb_auto_align( c_user_cmd* cmd );
		void texture_bug( c_user_cmd* cmd );
		void head_bounce( c_user_cmd* cmd );
		void preserve_move( c_user_cmd* cmd );
		void reset_command( ) { m_owned_move.clear( ); m_wallstrafed = false; }

		bool m_hit      = false;
		bool m_hs_hit   = false;
		bool m_hb_hit   = false; // head bounce landed this tick, one tick only, never a ride
		bool m_acted    = false;
		bool m_owns_cmd = false;
		bool m_assist   = false;
		bool m_wallstrafed = false;
		bool m_resimmed      = false;
		n_tb::wallstrafe::owned_move_t m_owned_move{ };
		float m_input_forward = 0.f, m_input_side = 0.f, m_input_yaw = 0.f;
	};
}

inline n_texturebug::impl_t g_texturebug{ };
