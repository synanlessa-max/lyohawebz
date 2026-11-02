#include "misc.h"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include "../../game/sdk/includes/includes.h"
#include "../../globals/includes/includes.h"
#include "../../globals/logger/logger.h"
#include "../avatar_cache/avatar_cache.h"
#include "../entity_cache/entity_cache.h"
#include "../movement/movement.h"
#include "../instalisation fonts/fonts2.h"
void n_misc::impl_t::on_create_move_pre( )
{
        this->disable_post_processing( );

        this->remove_panorama_blur( );

        this->practice_window_think( );
}

void n_misc::impl_t::on_paint_traverse( )
{
        this->draw_spectating_local( );
        
        
#ifdef _DEBUG
        // TESTING FUNCTION
        [ & ]( const bool run ) {
                if ( !run || !GET_VARIABLE( g_variables.m_debugger_visual, bool ) )
                        return;

                float offset = 0.f;

                static auto render_jb_debug = [ & ]( const std::string& text ) {
                        g_render.m_draw_data.emplace_back( e_draw_type::draw_type_text, std::make_any< text_draw_object_t >(
                                                                                                                                                                g_render.m_fonts[ e_font_names::font_name_tahoma_bd_12 ],
                                                                                                                                                                c_vector_2d( g_ctx.m_width / 2, ( g_ctx.m_height / 1.5 ) + offset ),
                                                                                                                                                                text.c_str( ), ImColor( 1.f, 1.f, 1.f, 1.f ),
                                                                                                                                                                ImColor( 0.f, 0.f, 0.f, 1.f ), e_text_flags::text_flag_dropshadow ) );
                        offset += 10;
                };

                render_jb_debug( std::string( "can_jb = " ).append( std::to_string( g_movement.m_jumpbug_data.m_can_jb ) ) );
                render_jb_debug( std::string( "height_diff = " ).append( std::to_string( g_movement.m_jumpbug_data.m_height_diff ) ) );
                render_jb_debug(
                        std::string( "vertical_velocity_at_landing = " ).append( std::to_string( g_movement.m_jumpbug_data.m_vertical_velocity_at_landing ) ) );
                render_jb_debug( std::string( "abs_height_diff = " ).append( std::to_string( g_movement.m_jumpbug_data.m_abs_height_diff ) ) );
                render_jb_debug( std::string( "ticks_till_land = " ).append( std::to_string( g_movement.m_jumpbug_data.m_ticks_till_land ) ) );
        }( GET_VARIABLE( g_variables.m_jump_bug, bool ) && g_input.check_input( &GET_VARIABLE( g_variables.m_jump_bug_key, key_bind_t ) ) );
#endif
}

static std::chrono::steady_clock::time_point progressStartTime = std::chrono::steady_clock::now( );
static double calculatedPositionMs                             = 0.0; // ïîçèöèÿ â ìèëëèñåêóíäàõ
static std::string lastTitle                                   = "";  // äëÿ îòñëåæèâàíèÿ ñìåíû òðåêà

void UpdateCalculatedTrackPosition( Player& player )
{
        // Åñëè ïðîèçîøëà ñìåíà òðåêà, ñáðàñûâàåì ïîçèöèþ è âðåìÿ ñòàðòà
        if ( lastTitle != player.Title ) {
                calculatedPositionMs = 0.0;
                progressStartTime    = std::chrono::steady_clock::now( );
                lastTitle            = player.Title;
        }

        // Åñëè òðåê âîñïðîèçâîäèòñÿ, ïðèáàâëÿåì ïðîøåäøåå âðåìÿ
        if ( player.isPlaying ) {
                auto now     = std::chrono::steady_clock::now( );
                auto deltaMs = std::chrono::duration_cast< std::chrono::milliseconds >( now - progressStartTime ).count( );
                calculatedPositionMs += deltaMs;
                progressStartTime = now;
        }
}



// Ïðèìåð îòðèñîâêè ïëååðà ñ èñïîëüçîâàíèåì ImGui è íîâîãî ðàñ÷¸òà ïðîãðåññà
void RenderMediaPlayer( )
{
        // Îáíîâëÿåì ðàññ÷èòàííóþ ïîçèöèþ òðåêà
        UpdateCalculatedTrackPosition( player );

        // Âû÷èñëÿåì ïðîãðåññ, èñïîëüçóÿ äëèòåëüíîñòü òðåêà (TotalTime â ìñ)
        float progress = 0.0f;
        if ( player.TotalTime > 0 ) {
                progress = static_cast< float >( calculatedPositionMs ) / static_cast< float >( player.TotalTime );
                if ( progress > 1.0f )
                        progress = 1.0f;
        }

        // Äîáàâëÿåì èíòåðïîëÿöèþ äëÿ ïëàâíîãî äâèæåíèÿ ïðîãðåññà
        static float smoothProgress = 0.0f;
        if ( progress < smoothProgress )
                smoothProgress = progress;
        smoothProgress += ( progress - smoothProgress ) * 0.1f; // êîýôôèöèåíò èíòåðïîëÿöèè (ìîæíî íàñòðîèòü)

        static ImVec2 sz{ };
        int x = g_ctx.m_width;
        int y = g_ctx.m_height;
        if ( GET_VARIABLE( g_variables.m_watermark, bool ) )
                ImGui::SetNextWindowPos( { x - sz.x - 10, 45 } );
        else
                ImGui::SetNextWindowPos( { x - sz.x - 10, 10 } );

        ImGui::Begin( "Media Player", nullptr,
                      ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize );
        ImGui::PushFont( fonts_for_gui::regular_11 );

        float padding    = 10.0f; // Îòñòóï ìåæäó òåêñòîì è èçîáðàæåíèåì
        float imageWidth = 30.0f; // Øèðèíà èçîáðàæåíèÿ

        // Ðàññ÷èòûâàåì ðàçìåðû òåêñòà
        auto sizex1       = ImGui::CalcTextSize( strartist.c_str( ) ).x;
        auto sizex2       = ImGui::CalcTextSize( strtitle.c_str( ) ).x;
        float windowWidth = ImGui::GetWindowSize( ).x;
        if ( albumArtTexture ) {
                ImGui::SetCursorPos( ImVec2( windowWidth - imageWidth - padding, 3 ) );
                ImGui::Image( albumArtTexture, ImVec2( imageWidth, imageWidth ) );
        }
        if ( albumArtTexture )
                ImGui::SetCursorPos( { windowWidth - imageWidth - padding - sizex1 - padding, 20 } );
        else
                ImGui::SetCursorPos( { windowWidth - padding - sizex1 - padding, 20 } );

        ImGui::TextColored( ImVec4( 0.f, 0.f, 0.f, 1.f ), strartist.c_str( ) );
        if ( albumArtTexture )
                ImGui::SetCursorPos( { windowWidth - imageWidth - padding - sizex1 - padding + 1, 19 } );
        else
                ImGui::SetCursorPos( { windowWidth - padding - sizex1 - padding + 1, 19 } );
        ImGui::TextColored( ImVec4( 0.6f, 0.6f, 0.6f, 1.f ), strartist.c_str( ) );
        if ( albumArtTexture )
                ImGui::SetCursorPos( { windowWidth - imageWidth - padding - sizex2 - padding, 5 } );
        else
                ImGui::SetCursorPos( { windowWidth - padding - sizex2 - padding, 5 } );

        ImGui::TextColored( ImVec4( 0.f, 0.f, 0.f, 1.f ), strtitle.c_str( ) );
        if ( albumArtTexture )
                ImGui::SetCursorPos( { windowWidth - imageWidth - padding - sizex2 - padding + 1, 4 } );
        else
                ImGui::SetCursorPos( { windowWidth - padding - sizex2 - padding + 1, 4 } );
        ImGui::TextColored( ImVec4( 1.f, 1.f, 1.f, 1.f ), strtitle.c_str( ) );

        if ( albumArtTexture ) {
                ImGui::PushItemWidth( 158 );
                ImGui::SetCursorPos( { sz.x - imageWidth - padding - 126, 40 } );
                ImGui::ProgressBar( smoothProgress, ImVec2( 0.0f, 2.0f ) );
                ImGui::PopItemWidth( );
        }
        ImGui::Spacing( );
        
        ImGui::SetWindowSize( { 400, 100 } );
        sz = ImGui::GetWindowSize( );

        ImGui::PopFont( );
        ImGui::End( );
}

// Watermark rendering: 7 styles + customizable text ( menu: Misc -> watermark )
// styles 0-1 are clean ( no effects ), 2-6 are animated
void RenderWatermark( )
{
    static auto width = 0.f;

    // custom text from the config ( single space fallback so the box never collapses )
    static std::string wm_text;
    wm_text = GET_VARIABLE( g_variables.m_watermark_text, std::string );
    if ( wm_text.empty( ) )
        wm_text = " ";
    const char* text = wm_text.c_str( );

    const int   style = GET_VARIABLE( g_variables.m_watermark_style, int );
    const float t     = static_cast< float >( ImGui::GetTime( ) );
    const float hue   = t * 0.12f;

    // hue -> ImVec4 ( wraps around 1.0 )
    auto hue_col = [ ] ( float h, float s, float v, float a ) {
        float r, g, b;
        ImGui::ColorConvertHSVtoRGB( h - floorf( h ), s, v, r, g, b );
        return ImVec4( r, g, b, a );
    };

    ImGui::SetNextWindowSize( ImVec2( -1, 29.f ), ImGuiCond_Always );
    ImGui::SetNextWindowPos( ImVec2( g_ctx.m_width - width - 5.f, 5.f ), ImGuiCond_Always );
    ImGui::Begin( "hotwheels-watermark", 0,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground );
    {
        const auto window    = ImGui::GetCurrentWindow( );
        const auto draw_list = window->DrawList;
        const auto size      = window->Size;
        const auto pos       = window->Pos;
        const ImVec2 box_min = pos;
        const ImVec2 box_max = ImVec2( pos.x + size.x, pos.y + size.y );

        ImGui::PushFont( fonts_for_gui::bolder_11 );

        const float line_h = ImGui::GetTextLineHeight( );
        const float text_w = ImGui::CalcTextSize( text ).x;

        // 0 classic: dark rounded box + outline, white text ( no effects )
        // 1 minimal: text with a soft shadow only ( no effects )
        // 2 gradient: animated multi-color banner, outlined white text
        // 3 rainbow: dark box, text hue cycles
        // 4 glow pulse: dark box, glowing text breathes
        // 5 neon: glowing cycling border + matching glowing text
        // 6 accent bar: dark box, cycling left bar + underline

        float text_x = pos.x + 10.f;

        // background per style
        switch ( style ) {
        default:
        case 0: // classic
            draw_list->AddRectFilled( box_min, box_max, IM_COL32( 20, 20, 25, 230 ), 10.0f );
            draw_list->AddRectFilled( ImVec2( box_min.x + 2.f, box_min.y + 2.f ), ImVec2( box_max.x - 2.f, box_max.y - 2.f ),
                IM_COL32( 40, 110, 150, 20 ), 9.0f );
            draw_list->AddRect( ImVec2( box_min.x + 1.f, box_min.y + 1.f ), ImVec2( box_max.x - 1.f, box_max.y - 1.f ),
                IM_COL32( 60, 60, 70, 255 ), 10.0f );
            break;
        case 1: // minimal - no background
            break;
        case 2: // gradient banner
        {
            const ImU32 c_l = ImGui::ColorConvertFloat4ToU32( hue_col( hue + 0.58f, 0.62f, 0.30f, 0.92f ) );
            const ImU32 c_m = ImGui::ColorConvertFloat4ToU32( hue_col( hue + 0.66f, 0.62f, 0.48f, 0.92f ) );
            const ImU32 c_r = ImGui::ColorConvertFloat4ToU32( hue_col( hue + 0.74f, 0.62f, 0.30f, 0.92f ) );
            draw_list->AddRectFilledMultiColor( box_min, box_max, c_l, c_m, c_r, c_m );
            draw_list->AddRect( box_min, box_max, IM_COL32( 0, 0, 0, 130 ), 0.0f );
            break;
        }
        case 3: // rainbow - classic dark box
        case 4: // glow pulse - classic dark box
            draw_list->AddRectFilled( box_min, box_max, IM_COL32( 20, 20, 25, 230 ), 10.0f );
            draw_list->AddRect( ImVec2( box_min.x + 1.f, box_min.y + 1.f ), ImVec2( box_max.x - 1.f, box_max.y - 1.f ),
                IM_COL32( 60, 60, 70, 255 ), 10.0f );
            break;
        case 5: // neon - dark box + layered glowing border
        {
            draw_list->AddRectFilled( box_min, box_max, IM_COL32( 12, 12, 16, 235 ), 10.0f );
            const float wave = 0.7f + 0.3f * sinf( t * 2.4f );
            for ( int i = 0; i < 4; ++i ) {
                const float grow  = static_cast< float >( i );
                const float alpha = 0.35f * ( 1.f - grow / 4.f ) * wave;
                const ImVec4 glow = hue_col( hue, 0.75f, 1.f, alpha );
                draw_list->AddRect( ImVec2( box_min.x - grow, box_min.y - grow ), ImVec2( box_max.x + grow, box_max.y + grow ),
                    ImGui::ColorConvertFloat4ToU32( glow ), 10.0f + grow, 0, 1.5f );
            }
            break;
        }
        case 6: // accent bar - dark box + cycling accent
        {
            draw_list->AddRectFilled( box_min, box_max, IM_COL32( 18, 18, 22, 235 ), 10.0f );
            draw_list->AddRect( ImVec2( box_min.x + 1.f, box_min.y + 1.f ), ImVec2( box_max.x - 1.f, box_max.y - 1.f ),
                IM_COL32( 55, 55, 62, 255 ), 10.0f );
            text_x += 6.f;
            const ImU32 acc = ImGui::ColorConvertFloat4ToU32( hue_col( hue, 0.80f, 1.f, 1.f ) );
            draw_list->AddRectFilled( ImVec2( box_min.x + 2.f, box_min.y + 4.f ), ImVec2( box_min.x + 5.f, box_max.y - 4.f ), acc, 1.5f );
            draw_list->AddRectFilled( ImVec2( box_min.x + 8.f, box_max.y - 3.f ), ImVec2( box_max.x - 8.f, box_max.y - 2.f ),
                ImGui::ColorConvertFloat4ToU32( hue_col( hue, 0.80f, 1.f, 0.35f ) ) );
            break;
        }
        }

        // text per style
        const float text_y = pos.y + ( size.y - line_h ) * 0.5f;
        auto add_text = [ & ] ( const ImVec2& p, ImU32 col ) {
            draw_list->AddText( ImGui::GetFont( ), ImGui::GetFontSize( ), p, col, text );
        };
        auto add_text_outlined = [ & ] ( const ImVec2& p, ImU32 col ) {
            add_text( ImVec2( p.x - 1.f, p.y ), IM_COL32( 0, 0, 0, 170 ) );
            add_text( ImVec2( p.x + 1.f, p.y ), IM_COL32( 0, 0, 0, 170 ) );
            add_text( ImVec2( p.x, p.y - 1.f ), IM_COL32( 0, 0, 0, 170 ) );
            add_text( ImVec2( p.x, p.y + 1.f ), IM_COL32( 0, 0, 0, 170 ) );
            add_text( p, col );
        };

        switch ( style ) {
        default:
        case 0: // classic + accent: plain white
        case 6:
            add_text( ImVec2( text_x, text_y ), IM_COL32( 255, 255, 255, 255 ) );
            break;
        case 1: // minimal: soft shadow + white
            add_text( ImVec2( text_x + 1.f, text_y + 1.f ), IM_COL32( 0, 0, 0, 150 ) );
            add_text( ImVec2( text_x, text_y ), IM_COL32( 255, 255, 255, 255 ) );
            break;
        case 2: // gradient: outlined white
            add_text_outlined( ImVec2( text_x, text_y ), IM_COL32( 255, 255, 255, 255 ) );
            break;
        case 3: // rainbow text
            add_text_outlined( ImVec2( text_x, text_y ),
                ImGui::ColorConvertFloat4ToU32( hue_col( t * 0.25f, 0.70f, 1.f, 1.f ) ) );
            break;
        case 4: // glow pulse
        {
            const float pulse  = 0.5f + 0.5f * sinf( t * 2.6f );
            const ImU32  glowc = ImGui::ColorConvertFloat4ToU32( hue_col( 0.55f, 0.65f, 1.f, 0.08f + 0.22f * pulse ) );
            for ( int i = 0; i < 4; ++i ) {
                static const ImVec2 offs[ 4 ] = { ImVec2( -1.f, 0.f ), ImVec2( 1.f, 0.f ), ImVec2( 0.f, -1.f ), ImVec2( 0.f, 1.f ) };
                add_text( ImVec2( text_x + offs[ i ].x, text_y + offs[ i ].y ), glowc );
            }
            add_text( ImVec2( text_x, text_y ), IM_COL32( 255, 255, 255, 255 ) );
            break;
        }
        case 5: // neon text ( same hue as the border )
        {
            const float wave  = 0.7f + 0.3f * sinf( t * 2.4f );
            const ImU32  glowc = ImGui::ColorConvertFloat4ToU32( hue_col( hue, 0.75f, 1.f, 0.35f * wave ) );
            for ( int i = 0; i < 4; ++i ) {
                static const ImVec2 offs[ 4 ] = { ImVec2( -1.f, 0.f ), ImVec2( 1.f, 0.f ), ImVec2( 0.f, -1.f ), ImVec2( 0.f, 1.f ) };
                add_text( ImVec2( text_x + offs[ i ].x, text_y + offs[ i ].y ), glowc );
            }
            add_text( ImVec2( text_x, text_y ), ImGui::ColorConvertFloat4ToU32( hue_col( hue, 0.30f, 1.f, 1.f ) ) );
            break;
        }
        }

        // keep the window auto-sized to the text ( draw list items do not size windows )
        ImGui::SetCursorPos( ImVec2( text_x - pos.x, text_y - pos.y ) );
        ImGui::Dummy( ImVec2( text_w, line_h ) );

        ImGui::PopFont( );
        width = ImGui::GetWindowSize( ).x;
    }
    ImGui::End( );
}

void n_misc::impl_t::on_end_scene( )
{
        if ( GET_VARIABLE( g_variables.trackdispay, bool ) )
                RenderMediaPlayer( );
        if (GET_VARIABLE(g_variables.m_watermark, bool))
                RenderWatermark();

        // functions that require player to be ingame
        if ( !g_ctx.m_local || !g_interfaces.m_engine_client->is_in_game( ) || g_ctx.m_local->get_observer_mode( ) == 1 /*DEATH_CAM*/ )
                return;

        // practice window
        [ & ]( const bool can_draw_practice_window ) {
                if ( !can_draw_practice_window )
                        return;
                if ( !g_ctx.m_local->is_alive( ) )
                        return;

                constexpr float window_width = 170.0f;
                constexpr float window_rounding = 7.0f;
                constexpr float window_padding = 0.0f;
                constexpr float title_top_margin = 5.0f;
                constexpr float first_bind_top = 25.0f;
                constexpr float bind_left_margin = 10.0f;
                constexpr float key_box_size = 20.0f;
                constexpr float key_box_rounding = 5.0f;
                constexpr float bind_row_height = 24.0f;
                constexpr float bind_row_spacing = 3.0f;
                constexpr float bottom_margin = 5.0f;
                constexpr ImU32 bg_color = IM_COL32(13,13,13,255);
                constexpr ImU32 key_box_border = IM_COL32(22,22,22,255);
                constexpr ImU32 key_text_color = IM_COL32(100,100,100,255);
                constexpr ImU32 label_color = IM_COL32(100,100,100,255);
                constexpr ImU32 title_color = IM_COL32(255,255,255,255);

                ImFont* key_font = fonts_for_gui::regular_11;
                float key_font_size = 10.f;
                float vertical_offset = -25.0f; // поднимаем бокс и текст чуть выше

                float window_height = first_bind_top + 2 * bind_row_height + bind_row_spacing + bottom_margin;

                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, window_rounding);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(window_padding, window_padding));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                ImGui::PushStyleColor(ImGuiCol_WindowBg, bg_color);

                ImGui::SetNextWindowSize(ImVec2(window_width, window_height), ImGuiCond_Always);
                int flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
                ImGui::Begin("Checkpoints", nullptr, flags);
                {
                        ImDrawList* draw_list = ImGui::GetWindowDrawList();
                        ImVec2 win_pos = ImGui::GetWindowPos();

                        // Заголовок по центру
                        ImGui::PushFont(fonts_for_gui::regular_11);
                        const char* title = "Checkpoints";
                        ImVec2 title_size = ImGui::CalcTextSize(title);
                        ImGui::SetCursorPosY(title_top_margin);
                        ImGui::SetCursorPosX((window_width - title_size.x) * 0.5f);
                        ImGui::TextColored(ImColor(title_color), title);
                        ImGui::PopFont();

                        // Массив подписей и биндов (только Save и Teleport)
                        struct bind_row_t { const char* label; const char* key; };
                        bind_row_t rows[2] = {
                                {"Save", FILTERED_KEY_NAMES[GET_VARIABLE(g_variables.m_practice_cp_key, key_bind_t).m_key]},
                                {"Teleport", FILTERED_KEY_NAMES[GET_VARIABLE(g_variables.m_practice_tp_key, key_bind_t).m_key]}
                        };

                        float y = first_bind_top;
                        for (int i = 0; i < 2; ++i) {
                                // Label
                                ImGui::SetCursorPosY(y);
                                ImGui::SetCursorPosX(bind_left_margin);
                                ImGui::PushFont(fonts_for_gui::regular_11);
                                ImGui::TextColored(ImColor(label_color), rows[i].label);
                                ImGui::PopFont();

                                // Key box — строго напротив текста, но чуть выше
                                ImVec2 label_pos = ImGui::GetCursorScreenPos(); // Сохраняем ДО вывода текста!
                                ImVec2 key_box_pos = ImVec2(win_pos.x + window_width - bind_left_margin - key_box_size, label_pos.y + vertical_offset);
                                draw_list->AddRect(
                                        key_box_pos,
                                        ImVec2(key_box_pos.x + key_box_size, key_box_pos.y + key_box_size),
                                        key_box_border,
                                        3.0f, // радиус скругления
                                        ImDrawFlags_RoundCornersAll, // скруглять все углы
                                        2.0f
                                );

                                // Key text — по центру бокса
                                std::string key_text = rows[i].key;
                                std::transform(key_text.begin(), key_text.end(), key_text.begin(), ::toupper);
                                ImVec2 key_text_size = key_font->CalcTextSizeA(key_font_size, FLT_MAX, 0.0f, key_text.c_str());
                                ImVec2 key_text_pos = ImVec2(
                                        key_box_pos.x + (key_box_size - key_text_size.x) * 0.5f,
                                        key_box_pos.y + (key_box_size - key_text_size.y) * 0.2f + 2.5f // <-- вот это!
                                );
                                draw_list->AddText(key_font, key_font_size, key_text_pos, key_text_color, key_text.c_str());

                                y += bind_row_height + bind_row_spacing;
                        }
                }
                ImGui::End();
                ImGui::PopStyleColor(1);
                ImGui::PopStyleVar(3);
        }( GET_VARIABLE( g_variables.m_practice_window, bool ) );

        [ & ]( bool can_draw_spectator_list, int type ) {
                if ( type == 0 && can_draw_spectator_list )
                        draw_spectator_list( );
        }( GET_VARIABLE( g_variables.m_spectators_list, bool ), GET_VARIABLE( g_variables.m_spectators_list_type, int ) );


        

}

void n_misc::impl_t::on_fire_event( ) {


}

void n_misc::impl_t::practice_window_think( )
{
        if ( !GET_VARIABLE( g_variables.m_practice_window, bool ) )
                return;

        if ( !g_convars[ HASH_BT( "sv_cheats" ) ]->get_bool( ) || g_interfaces.m_engine_client->is_console_visible( ) )
                return;

        const auto cp_key = GET_VARIABLE( g_variables.m_practice_cp_key, key_bind_t ).m_key;

        const auto tp_key = GET_VARIABLE( g_variables.m_practice_tp_key, key_bind_t ).m_key;

        if ( g_input.is_key_released( cp_key ) ) {
                if ( !( g_ctx.m_local->get_flags( ) & fl_onground ) ) {
                        g_logger.print( "you need to be onground to set a checkpoint.", "[practice]" );
                        return;
                }

                c_angle saved_angles = { };
                g_interfaces.m_engine_client->get_view_angles( saved_angles );

                g_misc.practice.saved_angles   = saved_angles;
                g_misc.practice.saved_position = g_ctx.m_local->get_abs_origin( );

                g_logger.print( "saved checkpoint.", "[practice]" );
        } else if ( g_input.is_key_released( tp_key ) ) {
                if ( g_misc.practice.saved_angles.is_zero( ) || g_misc.practice.saved_position.is_zero( ) )
                        return;

                g_interfaces.m_engine_client->client_cmd_unrestricted(
                        std::string( "setpos " )
                                .append( std::vformat( "{} {} {}", std::make_format_args( g_misc.practice.saved_position.m_x, g_misc.practice.saved_position.m_y,
                                                                                  g_misc.practice.saved_position.m_z ) ) )
                                .append( ";setang " )
                                .append( std::vformat( "{} {} {}", std::make_format_args( g_misc.practice.saved_angles.m_x, g_misc.practice.saved_angles.m_y,
                                                                                  g_misc.practice.saved_angles.m_z ) ) )
                                .c_str( ) );
        }
}

void n_misc::impl_t::disable_post_processing( )
{
        g_convars[ HASH_BT( "mat_postprocess_enable" ) ]->set_value( !( GET_VARIABLE( g_variables.m_disable_post_processing, bool ) ) );
}

void n_misc::impl_t::remove_panorama_blur( )
{
        g_convars[ HASH_BT( "@panorama_disable_blur" ) ]->set_value( GET_VARIABLE( g_variables.m_remove_panorama_blur, bool ) );
}

void n_misc::impl_t::draw_spectating_local( )
{
        int m_y = 5;

        std::vector< spectator_data_t > spectator_data{ };

        if ( !g_ctx.m_local || !g_interfaces.m_engine_client->is_in_game( ) || !GET_VARIABLE( g_variables.m_spectators_list, bool ) ||
             GET_VARIABLE( g_variables.m_spectators_list_type, int ) != 1 /*list local spectators*/ ) {
                if ( !spectator_data.empty( ) ) {
                        g_ctx.m_last_spectators_y = 5;
                        spectator_data.clear( );
                }
                return;
        }

        constexpr auto get_player_spec_type = [ & ]( int obs_mode ) -> std::string {
                switch ( obs_mode ) {
                case e_obs_mode::obs_mode_deathcam:
                        return "deathcam";
                case e_obs_mode::obs_mode_freezecam:
                        return "freezecam";
                case e_obs_mode::obs_mode_in_eye:
                        return "first person";
                case e_obs_mode::obs_mode_chase:
                        return "3rd person";
                case e_obs_mode::obs_mode_roaming:
                        return "roaming";
                case e_obs_mode::obs_mode_fixed:
                        return "fixed";
                default:
                        return "first person";
                }
        };

        g_entity_cache.enumerate( e_enumeration_type::type_players, [ & ]( c_base_entity* entity ) {
                if ( !entity || entity->is_alive( ) || entity->is_dormant( ) )
                        return;

                const auto entity_index = entity->get_index( );
                const auto entity_team  = entity->get_team( );

                const auto spectated_player = reinterpret_cast< c_base_entity* >(
                        g_interfaces.m_client_entity_list->get_client_entity_from_handle( entity->get_observer_target_handle( ) ) );

                if ( !spectated_player || spectated_player != g_ctx.m_local )
                        return;

                player_info_t spectating_info{ };
                g_interfaces.m_engine_client->get_player_info( entity_index, &spectating_info );

                if ( spectating_info.m_is_hltv )
                        return;

                spectator_data.push_back(
                        { std::format( "{} | {}",  std::string( spectating_info.m_name ).substr( 0, 12 ).append( "..." ),
                                                                      get_player_spec_type( entity->get_observer_mode( ) )  ),
                      spectating_info.m_fake_player ? entity_team == e_team_id::team_tt   ? g_render.m_terrorist_avatar
                                                      : entity_team == e_team_id::team_ct ? g_render.m_counter_terrorist_avatar
                                                                                          : nullptr
                                                    : g_avatar_cache[ entity_index ],
                      GET_VARIABLE( g_variables.m_spectators_list_text_color_one, c_color ) } );
        } );

        if ( spectator_data.empty( ) ) {
                g_ctx.m_last_spectators_y = 5;
                return;
        }

        const auto draw_avatar            = GET_VARIABLE( g_variables.m_spectators_avatar, bool );
        constexpr static auto avatar_size = 14.f;

        for ( const auto& data : spectator_data ) {
                if ( draw_avatar )
                        g_render.m_draw_data.emplace_back(
                                e_draw_type::draw_type_texture,
                                std::make_any< texture_draw_object_t >( c_vector_2d( 10, m_y ), c_vector_2d( avatar_size, avatar_size ),
                                                                    ImColor( 1.f, 1.f, 1.f, 1.f ), data.m_avatar, 0.f, ImDrawFlags_::ImDrawFlags_None ) );

                g_render.m_draw_data.emplace_back(
                        e_draw_type::draw_type_text,
                        std::make_any< text_draw_object_t >( g_render.m_fonts[ e_font_names::font_name_tahoma_bd_12 ], c_vector_2d( draw_avatar ? 27 : 10, m_y ),
                                                         data.m_text.c_str( ), data.m_color.get_u32( ),
                                                         ImColor( 0.f, 0.f, 0.f, data.m_color.base< e_color_type::color_type_a >( ) ),
                                                         e_text_flags::text_flag_dropshadow ) );

                m_y += 15;

                g_ctx.m_last_spectators_y = m_y;
        }
}

void n_misc::impl_t::draw_spectator_list( )
{
        if ( !g_ctx.m_local || !g_interfaces.m_engine_client->is_in_game( ) || !GET_VARIABLE( g_variables.m_spectators_list, bool ) )
                return;

        // panel layout (reference screenshot): dark double-framed box, centered "spectators" title,
        // colored line under the title (changeable in menu), inner black list box
        constexpr float window_width      = 200.0f;
        constexpr float side_margin       = 8.0f;  /* line & list box inset from panel sides */
        constexpr float title_top_margin  = 5.0f;
        constexpr float title_height      = 13.0f;
        constexpr float line_gap_top      = 4.0f;  /* gap between title and line */
        constexpr float line_height       = 3.0f;
        constexpr float list_gap_top      = 6.0f;  /* gap between line and list box */
        constexpr float list_min_height   = 26.0f; /* empty list height, like on the reference */
        constexpr float row_height        = 15.0f;
        constexpr float list_padding      = 6.0f;
        constexpr float bottom_padding    = 8.0f;
        constexpr ImU32 panel_bg          = IM_COL32( 16, 16, 16, 255 );
        constexpr ImU32 panel_border      = IM_COL32( 90, 90, 90, 255 );
        constexpr ImU32 panel_border_dark = IM_COL32( 35, 35, 35, 255 );
        constexpr ImU32 list_bg           = IM_COL32( 0, 0, 0, 235 );
        constexpr ImU32 list_border       = IM_COL32( 60, 60, 60, 255 );
        constexpr ImU32 text_color        = IM_COL32( 235, 235, 235, 255 );

        ImFont* const font = fonts_for_gui::regular_11;
        if ( !font )
                return;

        const float font_size = font->FontSize;
        const ImU32 line_color = GET_VARIABLE( g_variables.m_spectators_list_line_color, c_color ).get_u32( );

        // collect spectators (people who watch the local player)
        std::vector< std::string > spectators{ };
        g_entity_cache.enumerate( e_enumeration_type::type_players, [ & ]( c_base_entity* entity ) {
                if ( !entity || entity->is_alive( ) || entity->is_dormant( ) )
                        return;

                const auto spectated_player = reinterpret_cast< c_base_entity* >(
                        g_interfaces.m_client_entity_list->get_client_entity_from_handle( entity->get_observer_target_handle( ) ) );

                if ( !spectated_player || !spectated_player->is_alive( ) || spectated_player != g_ctx.m_local )
                        return;

                player_info_t info{ };
                g_interfaces.m_engine_client->get_player_info( entity->get_index( ), &info );

                if ( info.m_is_hltv )
                        return;

                spectators.push_back( std::string( info.m_name ).substr( 0, 24 ) );
        } );

        // list box grows with the amount of spectators
        const float list_height = std::max( list_min_height, list_padding * 2.0f + spectators.size( ) * row_height - 3.0f );

        const float window_height =
                title_top_margin + title_height + line_gap_top + line_height + list_gap_top + list_height + bottom_padding;

        // top-right corner; stack below the watermark / media player if those are enabled
        float y_offset = 5.0f;
        if ( GET_VARIABLE( g_variables.m_watermark, bool ) )
                y_offset += 34.0f; /* watermark 29px + 5px gap */
        if ( GET_VARIABLE( g_variables.trackdispay, bool ) )
                y_offset += 60.0f; /* media player approx height */

        ImGui::PushStyleVar( ImGuiStyleVar_WindowRounding, 0.0f );
        ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ) );
        ImGui::PushStyleVar( ImGuiStyleVar_WindowBorderSize, 0.0f );
        ImGui::PushStyleColor( ImGuiCol_WindowBg, IM_COL32( 0, 0, 0, 0 ) );

        ImGui::SetNextWindowSize( ImVec2( window_width, window_height ), ImGuiCond_Always );
        ImGui::SetNextWindowPos( ImVec2( static_cast< float >( g_ctx.m_width ) - window_width - 10.0f, y_offset ), ImGuiCond_Always );

        constexpr int window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
                                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground |
                                     ImGuiWindowFlags_NoSavedSettings;

        ImGui::Begin( "Spectators", nullptr, window_flags );
        {
                ImDrawList*  draw_list  = ImGui::GetWindowDrawList( );
                const ImVec2 panel_pos  = ImGui::GetWindowPos( );
                const ImVec2 panel_size = ImGui::GetWindowSize( );

                // panel background + double frame (light outer line + dark inner line)
                draw_list->AddRectFilled( panel_pos, ImVec2( panel_pos.x + panel_size.x, panel_pos.y + panel_size.y ), panel_bg );
                draw_list->AddRect( ImVec2( panel_pos.x + 1.0f, panel_pos.y + 1.0f ),
                                    ImVec2( panel_pos.x + panel_size.x - 1.0f, panel_pos.y + panel_size.y - 1.0f ), panel_border );
                draw_list->AddRect( ImVec2( panel_pos.x + 3.0f, panel_pos.y + 3.0f ),
                                    ImVec2( panel_pos.x + panel_size.x - 3.0f, panel_pos.y + panel_size.y - 3.0f ), panel_border_dark );

                // title
                const char*  title    = "spectators";
                const ImVec2 title_sz = font->CalcTextSizeA( font_size, FLT_MAX, 0.0f, title );
                const float  title_y  = panel_pos.y + title_top_margin;
                draw_list->AddText( font, font_size, ImVec2( panel_pos.x + ( panel_size.x - title_sz.x ) * 0.5f, title_y ), text_color, title );

                // colored line below the title
                const float line_y = title_y + title_height + line_gap_top;
                draw_list->AddRectFilled( ImVec2( panel_pos.x + side_margin, line_y ),
                                          ImVec2( panel_pos.x + panel_size.x - side_margin, line_y + line_height ), line_color );

                // inner list box
                const ImVec2 list_min( panel_pos.x + side_margin, line_y + line_height + list_gap_top );
                const ImVec2 list_max( panel_pos.x + panel_size.x - side_margin, list_min.y + list_height );
                draw_list->AddRectFilled( list_min, list_max, list_bg );
                draw_list->AddRect( list_min, list_max, list_border );

                // spectator names, centered
                draw_list->PushClipRect( list_min, list_max, true );

                float name_y = list_min.y + list_padding;

                for ( const auto& spectator : spectators ) {
                        std::string display_name = spectator;
                        const float max_width    = ( list_max.x - list_min.x ) - 8.0f;

                        while ( font->CalcTextSizeA( font_size, FLT_MAX, 0.0f, display_name.c_str( ) ).x > max_width && display_name.size( ) > 1U )
                                display_name.pop_back( );

                        const ImVec2 name_sz = font->CalcTextSizeA( font_size, FLT_MAX, 0.0f, display_name.c_str( ) );
                        draw_list->AddText( font, font_size,
                                            ImVec2( list_min.x + ( ( list_max.x - list_min.x ) - name_sz.x ) * 0.5f, name_y ), text_color,
                                            display_name.c_str( ) );

                        name_y += row_height;
                }

                draw_list->PopClipRect( );
        }
        ImGui::End( );

        ImGui::PopStyleColor( 1 );
        ImGui::PopStyleVar( 3 );
}
