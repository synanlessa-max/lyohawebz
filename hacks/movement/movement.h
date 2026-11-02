#pragma once
#define WINDOWS_IGNORE_PACKING_MISMATCH
#include <vector>
#include <Windows.h>
#include <string>
#include "../../game/sdk/classes/c_vector.h"
class c_user_cmd;
class c_angle;
class c_vector;

enum edgebug_type_t : int {
	eb_standing = 0,
	eb_ducking,
};

namespace n_movement
{
	struct impl_t {
		struct edgebug_data_t {
			edgebug_type_t m_edgebug_method{ };

			bool m_will_edgebug{ };
			bool m_will_fail{ };
			bool m_strafing{ };

			float m_yaw_delta{ };
			float m_starting_yaw{ };

			float m_side_move{ };
			float m_forward_move{ };

			float m_saved_mousedx{ };
			int m_ticks_to_stop{ };
			int m_last_tick{ };

			void reset( );
		} m_edgebug_data;

		struct jumpbug_data_t {
			int m_height_diff                    = 0.f;
			float m_vertical_velocity_at_landing = 0.f;
			float m_abs_height_diff              = 0.f;
			int m_ticks_till_land                = 0;
			bool m_can_jb                        = false;
		
			
          
		} m_jumpbug_data;

		struct pixelsurf_data_t {
			bool m_predicted_succesful = false, m_in_pixel_surf = false, m_should_duck = false;
			int m_prediction_ticks      = 0;
			c_user_cmd* m_simulated_cmd = { };
			bool should_pixel_surf      = false;
			void reset( );
		} m_pixelsurf_data;

		struct autoduck_data_t {
			bool m_did_land_ducking  = false;
			bool m_did_land_standing = false;

			float m_ducking_vert  = 0.f;
			float m_standing_vert = 0.f;

			void reset( );
		} m_autoduck_data;

                struct auto_bounce_data_t {
                        bool m_owns_cmd = false;
                        bool m_found    = false;
                        bool m_keep     = false;
                        int m_armed_tick = 0;
                        float m_entry_speed = 0.f;
                        float m_gain   = 0.f;
                        float m_angle  = 0.f;
                        float m_result = 0.f;
                        c_vector m_target{ };
                        c_vector m_normal{ };
                        int m_last_cmd     = 0;
                        int m_clip_cmd     = 0;
                        float m_last_vz    = 0.f;
                        float m_last_speed = 0.f;
                        bool m_jump_owed = false;
                        int m_refused    = 0;

                        /* lobo: bounce sound watcher state ( survives reset( ) - manages its own lifecycle ) */
                        int m_watch_cmd = 0;        /* last cmd the watcher saw */
                        float m_watch_vz = 0.f;     /* vz at that cmd */
                        bool m_watch_air = false;   /* that cmd was airborne */
                        int m_watch_armed_at = 0;   /* last tick the plan was armed */
                        int m_watch_fired_cmd = 0;  /* cmd the sound last fired on ( anti double fire ) */
                        float m_bounce_flash = 0.f; /* seconds left of the ab indicator success flash */
                        static constexpr int k_viz_max = 300;
                        c_vector m_viz[ k_viz_max ]{ };
                        int m_viz_n = 0;
                        c_vector m_viz_land{ };
                        c_vector m_viz_normal{ };

                        void disarm( )
                        {
                                m_viz_n       = 0;
                                m_owns_cmd    = false;
                                m_found       = false;
                                m_keep        = false;
                                m_armed_tick  = 0;
                                m_entry_speed = 0.f;
                                m_gain        = 0.f;
                                m_angle       = 0.f;
                                m_result      = 0.f;
                                m_target      = c_vector( );
                                m_normal      = c_vector( );
                                m_refused     = 0;
                        }

                        void reset( )
                        {
                                disarm( );
                                m_last_cmd   = 0;
                                m_clip_cmd   = 0;
                                m_last_vz    = 0.f;
                                m_last_speed = 0.f;
                                m_jump_owed  = false;
                        }
                } m_auto_bounce_data;

                struct fireman_data_t {
			bool is_ladder = false;
			bool fr_hit_1  = false;
			bool fr_hit    = false;
			bool awall     = false;
		} m_fireman_data;

		struct pixelsurf_assist_t {
			bool in_crosshair = false;
			bool set_point    = false;
			

		} m_pixelsurf_assist_t;
		
		struct ladder_bug_t {
			bool founded = false;
		} m_ladder_bug_data;

		struct air_stuck_t {
			bool m_air_stuck = false;
		
		} m_air_stuck_data;
		void on_create_move_pre( );
		void on_create_move_post( );
		void on_paint_traverse( );
		void on_frame_stage_notify( int stage );
		void on_end_scene( );
		void rotate_movement( c_user_cmd* cmd, c_angle& angle );

		void play_bounce_sound( );

		void pixel_finder( c_user_cmd* cmd );
	private:
		void bunny_hop( );

		void edge_jump( );

		void air_stuck( c_user_cmd* cmd );

		void edge_bug( );

		void long_jump( );

		void mini_jump( );

		void jump_bug( );

		void auto_duck( );

		void pixel_surf_locking( float target_ps_velocity );

		void pixel_surf_fix( );

		void pixel_surf( float target_ps_velocity );

		void detect_edgebug( c_user_cmd* cmd );

		void auto_align( c_user_cmd* cmd );

		void strafe_to_yaw( c_user_cmd* cmd, c_angle& angle, const float yaw );

		void movement_fix( const c_angle& old_view_point );

		void autobounce_assist( c_user_cmd* cmd );

		void auto_bounce( c_user_cmd* cmd, bool yield = false );

		void auto_bounce_render( );

		void pixelsurf_assist( c_user_cmd* cmd );
		
		void pixelsurf_assist_render( );

		void fire_man( c_user_cmd* cmd );
		void ladder_bug( c_user_cmd* cmd );
		void anti_ladder( c_user_cmd* cmd );
	};
} // namespace n_movement



inline n_movement::impl_t g_movement{ };
