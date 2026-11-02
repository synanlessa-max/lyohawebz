#define IMGUI_DEFINE_MATH_OPERATORS
#include "menu.h"
#include "../../globals/fonts/fonts.h"
#include "../../globals/includes/includes.h"
#include "../../globals/logger/logger.h"
#include "../misc/scaleform/scaleform.h"
#include "../movement/movement.h"
#include "../movement/edgebug.h"
#include "../../dependencies/imgui/imgui.h"
#include "../skins/skins.h"
#include "../elements/tabs.hh"
#include "../elements/childbox.hh"
#include "../elements/checkbox.hh"
#include "../elements/button.hh"
#include "../elements/sliders.hh"
#include "../elements/spinner.hh"
#include "../elements/combobox.hh"
#include "../elements/i_text.hh"
#include "../aimbot/aimbot.h"
constexpr int color_picker_alpha_flags = ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_AlphaPreviewHalf |
                                         ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_NoInputs |
                                         ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop | ImGuiColorEditFlags_PickerHueBar |
                                         ImGuiColorEditFlags_NoBorder;

constexpr int color_picker_no_alpha_flags = ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_InputRGB |
                                            ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoDragDrop |
                                            ImGuiColorEditFlags_PickerHueBar | ImGuiColorEditFlags_NoBorder;

constexpr static const auto chams_materials = "flat\0textured\0metallic\0glow";

void save_popup( const char* str_id, const ImVec2& window_size, const std::function< void( ) >& fn )
{
        ImGuiContext& g         = *GImGui;
        const ImGuiStyle& style = g.Style;

        const auto window = g.CurrentWindow;

        const auto hashed_str_id = ImHashStr( str_id );
        const auto text_size     = g_render.m_fonts[ e_font_names::font_name_verdana_bd_11 ]->CalcTextSizeA(
        g_render.m_fonts[ e_font_names::font_name_verdana_bd_11 ]->FontSize, FLT_MAX, 0.f, str_id );

        const ImColor accent_color = ImGui::GetColorU32( ImGuiCol_::ImGuiCol_Accent );

        ImGui::OpenPopup( str_id );

        ImGui::SetNextWindowSize( window_size );

        if ( ImGui::BeginPopup( str_id ) ) {
                const auto draw_list = ImGui::GetWindowDrawList( );

                const ImVec2 position = ImGui::GetWindowPos( ), size = ImGui::GetWindowSize( );

                auto text_animation = ImAnimationHelper( hashed_str_id, ImGui::GetIO( ).DeltaTime );

                ImGui::PushClipRect( ImVec2( position ), ImVec2( position.x + size.x, position.y + 20.f ), false );

                draw_list->AddRectFilled( ImVec2( position ), ImVec2( position.x + size.x, position.y + 20.f ), ImColor( 25 / 255.f, 25 / 255.f, 25 / 255.f ),
                                          g.Style.WindowRounding - 2.f, ImDrawFlags_RoundCornersTop );

                ImGui::PopClipRect( );

                ImGui::PushClipRect( position, ImVec2( position.x + size.x, position.y + size.y ), false );
                draw_list->AddRect( position, ImVec2( position.x + size.x, position.y + size.y ), ImColor( 50, 50, 50, 100 ), g.Style.WindowRounding - 2.f );

                text_animation.Update( ImGui::IsMouseHoveringRect( position, ImVec2( position.x + size.x, position.y + size.y ) ) ? 3.f : -2.f, 1.f, 0.5f,
                                       1.f );

                ImGui::PopClipRect( );

                const ImColor text_color = ImGui::GetColorU32( ImGuiCol_Text );

                draw_list->AddText(
                        g_render.m_fonts[ e_font_names::font_name_verdana_bd_11 ], g_render.m_fonts[ e_font_names::font_name_verdana_bd_11 ]->FontSize,
                        ImVec2( position.x + ( size.x - text_size.x ) / 2.f, position.y + ( 20.f - text_size.y ) / 2.f ),
                        ImColor( text_color.Value.x, text_color.Value.y, text_color.Value.z, text_color.Value.w * text_animation.AnimationData->second ),
                        str_id );

                RenderFadedGradientLine( draw_list, ImVec2( position.x, position.y + 20.f ), ImVec2( size.x, 1.f ),
                                         ImColor( accent_color.Value.x, accent_color.Value.y, accent_color.Value.z ) );

                ImGui::SetCursorPosY( ImGui::GetCursorPosY( ) + 20.f );

                fn( );

                ImGui::EndPopup( );
        }
}


extern void loadConfig( const std::string& filename );
extern void saveConfig( const std::string& filename );
extern void loadConfig2( const std::string& filename );
extern void saveConfig2( const std::string& filename );
constexpr std::array skyboxList{ "Default",
                                     "cs_baggage_skybox_",
                                     "cs_tibet",
                                     "embassy",
                                     "italy",
                                     "jungle",
                                     "nukeblank",
                                     "office",
                                     "sky_cs15_daylight01_hdr",
                                     "sky_cs15_daylight02_hdr",
                                     "sky_cs15_daylight03_hdr",
                                     "sky_cs15_daylight04_hdr",
                                     "sky_csgo_cloudy01",
                                     "sky_csgo_night_flat",
                                     "sky_csgo_night02",
                                     "sky_day02_05_hdr",
                                     "sky_day02_05",
                                     "sky_dust",
                                     "sky_l4d_rural02_ldr",
                                     "sky_venice",
                                     "vertigo_hdr",
                                     "vertigo",
                                     "vertigoblue_hdr",
                                     "vietnam",
                                     "sky_lunacy",
                                     "sky_hr_aztec",
                                     "Custom" };
extern bool point_menu_is_opened( );



ImColor RarityGetter( int rarity )
{
        if ( rarity == 1 )
                return ImColor( 0.69f, 0.76f, 0.85f, float( 1.f * g_menu.m_animation_progress ) );
        else if ( rarity == 2 )
                return ImColor( 0.29f, 0.41f, 1.f, float( 1.f * g_menu.m_animation_progress ) );
        else if ( rarity == 3 )
                return ImColor( 0.36f, 0.60f, 0.85f, float( 1.f * g_menu.m_animation_progress ) );
        else if ( rarity == 4 )
                return ImColor( 0.53f, 0.27f, 1.f, float( 1.f * g_menu.m_animation_progress ) );
        else if ( rarity == 5 )
                return ImColor( 0.82f, 0.17f, 0.90f, float( 1.f * g_menu.m_animation_progress ) );
        else if ( rarity == 6 )
                return ImColor( 0.92f, 0.29f, 0.29f, float( 1.f * g_menu.m_animation_progress ) );
        else if ( rarity == 7 )
                return ImColor( 0.89f, 0.68f, 0.22f, float( 1.f * g_menu.m_animation_progress ) );
}

std::string ToUtf888( std::wstring_view wstr )
{
        if ( wstr.empty( ) )
                return std::string( );

        auto size_needed = WideCharToMultiByte( CP_UTF8, 0, &wstr[ 0 ], static_cast< int32_t >( wstr.size( ) ), NULL, 0, NULL, NULL );

        std::string out( size_needed, 0 );
        WideCharToMultiByte( CP_UTF8, 0, &wstr[ 0 ], static_cast< int >( wstr.size( ) ), &out[ 0 ], size_needed, NULL, NULL );

        return out;
}

std::string LocalizeTex22( const char* in )
{
        const auto wide_name = g_interfaces.m_localize->find_safe( in );

        if ( !wcslen( wide_name ) )
                return "";

        return ToUtf888( wide_name );
}

std::string ToUtf88( std::wstring_view wstr )
{
        if ( wstr.empty( ) )
                return std::string( );

        auto size_needed = WideCharToMultiByte( CP_UTF8, 0, &wstr[ 0 ], static_cast< int32_t >( wstr.size( ) ), NULL, 0, NULL, NULL );

        std::string out( size_needed, 0 );
        WideCharToMultiByte( CP_UTF8, 0, &wstr[ 0 ], static_cast< int >( wstr.size( ) ), &out[ 0 ], size_needed, NULL, NULL );

        return out;
}
std::string LocalizeTex2( const char* in )
{
        const auto wide_name = g_interfaces.m_localize->find_safe( in );

        if ( !wcslen( wide_name ) )
                return "";

        return ToUtf88( wide_name );
}



void n_menu::impl_t::on_end_scene( )
{
        ImGui::GetStyle( ).Colors[ ImGuiCol_::ImGuiCol_Accent ] = GET_VARIABLE( g_variables.m_accent, c_color ).get_vec4( );

        // Обновление прогресса анимации (оставляем как есть)
        float delta_time = ImGui::GetIO( ).DeltaTime;
        if ( m_opened && m_animation_progress < 1.0f ) {
                m_is_opening = true;
                m_animation_progress += delta_time * m_animation_speed;
        } else if ( !m_opened && !point_menu_is_opened( ) && m_animation_progress > 0.0f ) {
                m_is_opening = false;
                m_animation_progress -= delta_time * m_animation_speed;
        }

        if ( point_menu_is_opened( ) ) {
                m_opened = true;
        }

        if ( point_menu_is_opened( ) && m_animation_progress < 1.0f ) {
                m_animation_progress += delta_time * m_animation_speed;
        }
        m_animation_progress = std::clamp( m_animation_progress, 0.0f, 1.0f );

        if ( m_animation_progress <= 0.0f || !m_opened || point_menu_is_opened( ) ) {
                return;
        }


        {
                auto& style                                   = ImGui::GetStyle( );
                style.Colors[ ImGuiCol_::ImGuiCol_WindowBg ]  = ImVec4( 10 / 255.f, 10 / 255.f, 10 / 255.f, float( 1.f * g_menu.m_animation_progress ) );
                style.Colors[ ImGuiCol_::ImGuiCol_ChildBg ]   = ImVec4( 15 / 255.f, 15 / 255.f, 15 / 255.f, float( 1.f * g_menu.m_animation_progress ) );
                style.Colors[ ImGuiCol_::ImGuiCol_FrameBg ]   = ImVec4( 25 / 255.f, 25 / 255.f, 25 / 255.f, float( 1.f * g_menu.m_animation_progress ) );
                style.Colors[ ImGuiCol_::ImGuiCol_PopupBg ]   = ImVec4( 20 / 255.f, 20 / 255.f, 20 / 255.f, float( 1.f * g_menu.m_animation_progress ) );
                style.Colors[ ImGuiCol_::ImGuiCol_CheckMark ] = ImVec4( 0 / 255.f, 0 / 255.f, 0 / 255.f, float( 1.f * g_menu.m_animation_progress ) );
                style.Colors[ ImGuiCol_::ImGuiCol_Button ]    = ImVec4( 20 / 255.f, 20 / 255.f, 20 / 255.f, float( 1.f * g_menu.m_animation_progress ) );

                style.Colors[ ImGuiCol_::ImGuiCol_Border ]       = ImVec4( 0 / 255.f, 0 / 255.f, 0 / 255.f, 0.f );
                style.Colors[ ImGuiCol_::ImGuiCol_BorderShadow ] = ImVec4( 0 / 255.f, 0 / 255.f, 0 / 255.f, 0.f );
                style.Colors[ ImGuiCol_Text ]                    = ImVec4( 230 / 255.f, 230 / 255.f, 230 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_TextDisabled ]            = ImVec4( 128 / 255.f, 128 / 255.f, 128 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_FrameBgHovered ]          = ImVec4( 30 / 255.f, 30 / 255.f, 30 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_FrameBgActive ]           = ImVec4( 35 / 255.f, 35 / 255.f, 35 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_TitleBg ]                 = ImVec4( 15 / 255.f, 15 / 255.f, 15 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_TitleBgActive ]           = ImVec4( 15 / 255.f, 15 / 255.f, 15 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_TitleBgCollapsed ]        = ImVec4( 15 / 255.f, 15 / 255.f, 15 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_MenuBarBg ]               = ImVec4( 20 / 255.f, 20 / 255.f, 20 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_ScrollbarBg ]             = ImVec4( 15 / 255.f, 15 / 255.f, 15 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_ScrollbarGrab ]           = ImVec4( 60 / 255.f, 60 / 255.f, 60 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_ScrollbarGrabHovered ]    = ImVec4( 80 / 255.f, 80 / 255.f, 80 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_ScrollbarGrabActive ]     = ImVec4( 100 / 255.f, 100 / 255.f, 100 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_SliderGrab ]              = ImVec4( 80 / 255.f, 80 / 255.f, 80 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_SliderGrabActive ]        = ImVec4( 100 / 255.f, 100 / 255.f, 100 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_ButtonHovered ]           = ImVec4( 25 / 255.f, 25 / 255.f, 25 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_ButtonActive ]            = ImVec4( 30 / 255.f, 30 / 255.f, 30 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_Header ] =
                        ImVec4( 20 / 255.f, 20 / 255.f, 20 / 255.f, 1.f * g_menu.m_animation_progress ); // Можно задать так же, как для Tab
                style.Colors[ ImGuiCol_HeaderHovered ]        = ImVec4( 50 / 255.f, 50 / 255.f, 50 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_HeaderActive ]         = ImVec4( 45 / 255.f, 45 / 255.f, 45 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_Separator ]            = ImVec4( 0 / 255.f, 0 / 255.f, 0 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_SeparatorHovered ]     = ImVec4( 60 / 255.f, 60 / 255.f, 60 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_SeparatorActive ]      = ImVec4( 80 / 255.f, 80 / 255.f, 80 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_ResizeGrip ]           = ImVec4( 80 / 255.f, 80 / 255.f, 80 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_ResizeGripHovered ]    = ImVec4( 100 / 255.f, 100 / 255.f, 100 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_ResizeGripActive ]     = ImVec4( 120 / 255.f, 120 / 255.f, 120 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_Tab ]                  = ImVec4( 20 / 255.f, 20 / 255.f, 20 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_TabHovered ]           = ImVec4( 40 / 255.f, 40 / 255.f, 40 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_TabActive ]            = ImVec4( 25 / 255.f, 25 / 255.f, 25 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_TabUnfocused ]         = ImVec4( 20 / 255.f, 20 / 255.f, 20 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_TabUnfocusedActive ]   = ImVec4( 25 / 255.f, 25 / 255.f, 25 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_PlotLines ]            = ImVec4( 100 / 255.f, 100 / 255.f, 100 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_PlotLinesHovered ]     = ImVec4( 120 / 255.f, 120 / 255.f, 120 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_PlotHistogram ]        = ImVec4( 150 / 255.f, 150 / 255.f, 150 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_PlotHistogramHovered ] = ImVec4( 170 / 255.f, 170 / 255.f, 170 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_TableHeaderBg ]        = ImVec4( 25 / 255.f, 25 / 255.f, 25 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_TableBorderStrong ]    = ImVec4( 30 / 255.f, 30 / 255.f, 30 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_TableBorderLight ]     = ImVec4( 25 / 255.f, 25 / 255.f, 25 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_TableRowBg ]           = ImVec4( 20 / 255.f, 20 / 255.f, 20 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_TableRowBgAlt ]        = ImVec4( 15 / 255.f, 15 / 255.f, 15 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_TextSelectedBg ]       = ImVec4( 40 / 255.f, 40 / 255.f, 40 / 255.f, 1.f * g_menu.m_animation_progress );
                // Нестандартный выбор цвета для эффекта DragDrop (можно менять по вкусу)
                style.Colors[ ImGuiCol_DragDropTarget ]        = ImVec4( 255 / 255.f, 255 / 255.f, 0 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_NavHighlight ]          = ImVec4( 80 / 255.f, 80 / 255.f, 80 / 255.f, 1.f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_NavWindowingHighlight ] = ImVec4( 255 / 255.f, 255 / 255.f, 255 / 255.f, 1.f * g_menu.m_animation_progress );
                // Для затемнения окон при навигации или модальных окнах можно задать меньшую альфу:
                style.Colors[ ImGuiCol_NavWindowingDimBg ] = ImVec4( 0 / 255.f, 0 / 255.f, 0 / 255.f, 0.5f * g_menu.m_animation_progress );
                style.Colors[ ImGuiCol_ModalWindowDimBg ]  = ImVec4( 0 / 255.f, 0 / 255.f, 0 / 255.f, 0.5f * g_menu.m_animation_progress );

                /* rounded frames, grabs and popups */
                style.FrameRounding     = 4.f;
                style.GrabRounding      = 4.f;
                style.PopupRounding     = 8.f;
                style.ScrollbarRounding = 6.f;
        }

        ImGui::SetNextWindowSize( ImVec2( 630.f, 500.f ), ImGuiCond_::ImGuiCond_Always );
        ImGui::Begin( "LilPeepXANAX", nullptr,
                      ImGuiWindowFlags_::ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_::ImGuiWindowFlags_NoBackground |
                          ImGuiWindowFlags_::ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse |
                          ImGuiWindowFlags_::ImGuiWindowFlags_NoResize | ImGuiWindowFlags_::ImGuiWindowFlags_NoBackground );
        {
                /* Отрисовка фона, обводки и иконки (оставляем как есть) */
                ImGui::PushClipRect( ImGui::GetWindowPos( ), ImGui::GetWindowPos( ) + ImGui::GetWindowSize( ), false );
                ImGui::GetBackgroundDrawList( )->AddRectFilled( ImGui::GetWindowPos( ), ImGui::GetWindowPos( ) + ImGui::GetWindowSize( ),
                                                                ImColor( 8, 8, 11, int(255 * m_animation_progress) ), 10.0f, ImDrawFlags_RoundCornersAll );
                ImGui::GetBackgroundDrawList( )->AddRect( ImGui::GetWindowPos( ), ImGui::GetWindowPos( ) + ImGui::GetWindowSize( ),
                                                          ImColor( 36, 36, 42, int( 255 * m_animation_progress ) ), 10.0f, ImDrawFlags_RoundCornersAll );
                /* accent hairline along the top edge */
                {
                        const auto wpos = ImGui::GetWindowPos( );
                        const auto wsz  = ImGui::GetWindowSize( );
                        ImGui::GetBackgroundDrawList( )->AddRectFilled( ImVec2( wpos.x + 12.f, wpos.y + 1.f ),
                                                                        ImVec2( wpos.x + wsz.x - 12.f, wpos.y + 3.f ),
                                                                        GET_VARIABLE( g_variables.m_accent, c_color ).get_u32( int( 200 * m_animation_progress ) ),
                                                                        2.f );
                }
                //ImGui::GetBackgroundDrawList( )->AddText( fonts_for_gui::title, fonts_for_gui::title->FontSize,
                //                                          ImGui::GetWindowPos( ) + ImVec2( 20.0f, 55.0f ),
                //      ImColor( GET_VARIABLE( g_variables.m_accent, c_color ).get_u32( int( 255 * m_animation_progress ) ) ), "A" );
                
                ImGui::GetBackgroundDrawList( )->AddText( fonts_for_gui::logo, fonts_for_gui::logo->FontSize, ImGui::GetWindowPos( ) + ImVec2( 45.0f, 40.0f ),
                        ImColor(255,255,255, int( 255 * m_animation_progress ) ), "a" );
                auto L_size = fonts_for_gui::logo->CalcTextSizeA( fonts_for_gui::logo->FontSize, FLT_MAX, 0.f, "a" );
        //      ImGui::GetBackgroundDrawList( )->AddText( fonts_for_gui::logo, fonts_for_gui::logo->FontSize,
                   //                                       ImGui::GetWindowPos( ) + ImVec2( 45.0f + L_size.x + 2.f, 40.0f ),
                 //                                         GET_VARIABLE( g_variables.m_accent, c_color ).get_u32( float( 1.f * m_animation_progress ) ), "B" );
                ImGui::PopClipRect( );

                // Определение вкладок
                std::vector< tab_t > tabs = { { "aimbot", "B", { "legit" } },
                                                  { "visuals", "A", { "entities", "game" } },
                                                  { "misc", "C", { "general" } },
                                                  { "movement", "O", { "general", "recorder" } },
                                                  { "inventory", "D", { "weapons", "player" } },
                                                  { "fonts", "I", { "general" } },
                                                  { "config", "G", { "general" } } };

                custom_tabs( tabs );

                ImGui::PushFont( fonts_for_gui::regular );

                // Вкладка "legit" (aimbot, tab_number == 0)
                if ( tab_number == 0 ) {
                        if ( subtab_number == 0 ) { // Под вкладка "legit"
                                static int index = 0;
                                ImGui::SetCursorPos( ImVec2( 150.0f, 10.0f ) );
                                child( "main", ImVec2( 230.0f, 310.0f ),
                                       []( ) { 
                                                const char* weapons[] = { "sniper", "rifle", "shotgun", "machinegun", "pistol" };
                                                combo( "weapon", index, weapons, 5 );

                                                checkbox( "enable aimbot", &g_aimbot.m_aimbot_settings[ index ].m_enable );
                                                checkbox( "enable silent", &g_aimbot.m_aimbot_settings[ index ].m_silent );
                                                checkbox( "enable recoil control", &g_aimbot.m_aimbot_settings[ index ].m_recoil_control );
                                                checkbox( "enable visibility check", &g_aimbot.m_aimbot_settings[ index ].m_visibility_check );
                                                checkbox( "enable backtrack", &GET_VARIABLE( g_variables.m_backtrack_enable, bool ) );
                                                checkbox( "enable extended backtrack", &GET_VARIABLE( g_variables.m_backtrack_extend, bool ) );
                                                if ( GET_VARIABLE( g_variables.m_backtrack_enable,bool) )
                                                        checkbox( "enable aim to backtrack", &g_aimbot.m_aimbot_settings[ index ].m_aim_to_backtrack );
                                                const char* hitboxes[] = { "head", "neck", "body", "legs" };
                                                multi_combo( "hitboxes", g_aimbot.m_aimbot_settings[ index ].m_hitboxes, hitboxes, 4 );
                                        
                                        
                                        } );

                                ImGui::SetCursorPos( ImVec2( 150.0f+240.f, 10.0f ) );
                                child( "configuration", ImVec2( 230.0f, 310.0f ), []( ) {
                                        // Пустое окно конфигурации (можно дополнить позже)
                                        slider_float( "fov", &g_aimbot.m_aimbot_settings[ index ].m_fov, 0.f, 180.f, "%.1f" );
                                        slider_float( "smooth", &g_aimbot.m_aimbot_settings[ index ].m_smooth, 1.f, 100.f, "%.1f" );
                                        slider_int( "backtrack time limit", &GET_VARIABLE( g_variables.m_backtrack_time_limit,int ), 0, 200, "%d" );
                                        slider_int( "ping adder", &GET_VARIABLE( g_variables.fakeLatencyAmount, int ), 0, 200, "%d" );
                                } );
                        }
                }


                // Вкладка "visuals" (tab_number == 1)
                if ( tab_number == 1 ) {
                        // Players ESP
                        if ( subtab_number == 0 ) {
                                ImGui::SetCursorPos( ImVec2( 150.0f, 10.0f ) );
                                child( "players esp", ImVec2( 230.0f, 380.0f ), []( ) {
                                        checkbox( "players", &GET_VARIABLE( g_variables.m_players, bool ) );
                                        

                                        if ( GET_VARIABLE( g_variables.m_players, bool ) ) {
                                                // Bounding Box popup
                                                checkbox( "bounding box", &GET_VARIABLE( g_variables.m_players_box, bool ), false, true, { 200, -1 }, []( ) {
                                                        checkbox( "corner bounding box", &GET_VARIABLE( g_variables.m_players_box_corner, bool ) );

                                                        checkbox( "bounding box outline", &GET_VARIABLE( g_variables.m_players_box_outline, bool ) );
                                                        ImGui::Spacing( );
                                                        ImGui::Text( "player bounding box color" );
                                                        ImGui::ColorEdit4( "##player bounding box color", &GET_VARIABLE( g_variables.m_players_box_color, c_color ),
                                                                           color_picker_alpha_flags );
                                                        if ( GET_VARIABLE( g_variables.m_players_box_outline, bool ) ) {
                                                                ImGui::Spacing( );
                                                                ImGui::Text( "player box outline color" );
                                                                ImGui::ColorEdit4( "##player bounding box outline color",
                                                                                   &GET_VARIABLE( g_variables.m_players_box_outline_color, c_color ), color_picker_alpha_flags );
                                                        }
                                                } );

                                                // Name popup
                                                checkbox( "name", &GET_VARIABLE( g_variables.m_players_name, bool ), false, true, { 200, -1 }, []( ) {
                                                        ImGui::Text( "player name color" );

                                                        ImGui::ColorEdit4( "##player name color", &GET_VARIABLE( g_variables.m_players_name_color, c_color ),
                                                                           color_picker_alpha_flags );
                                                } );

                                                // Health bar popup
                                                checkbox( "health bar", &GET_VARIABLE( g_variables.health_bar_enable, bool ), false, true, { 200.f, -1.f }, []( ) {
                                                        checkbox( "gradient color", &GET_VARIABLE( g_variables.health_bar_gradient, bool ) );

                                                        checkbox( "color based on health", &GET_VARIABLE( g_variables.health_bar_baseonhealth, bool ) );
                                                        checkbox( "fill outline background", &GET_VARIABLE( g_variables.health_bar_background, bool ) );
                                                        checkbox( "text on bar", &GET_VARIABLE( g_variables.health_bar_text, bool ) );
                                                        slider_int( "bar width", &GET_VARIABLE( g_variables.health_bar_size, int ), 1, 10, "%.d" );
                                                        ImGui::Spacing( );
                                                        ImGui::Text( "healthbar outline color" );
                                                        ImGui::ColorEdit4( "##healthbar outline", &GET_VARIABLE( g_variables.colors_health_bar_outline, c_color ) );
                                                        if ( GET_VARIABLE( g_variables.colors_custom, bool ) ) {
                                                                ImGui::Spacing( );
                                                                ImGui::Text( "healthbar color" );
                                                                ImGui::ColorEdit4( "##healthbar color", &GET_VARIABLE( g_variables.colors_health_bar, c_color ) );
                                                        }
                                                        if ( GET_VARIABLE( g_variables.health_bar_gradient, bool ) ) {
                                                                ImGui::Spacing( );
                                                                ImGui::Text( "healthbar upper color" );
                                                                ImGui::Spacing( );
                                                                ImGui::ColorEdit4( "##healthbar upper color", &GET_VARIABLE( g_variables.colors_health_bar_upper, c_color ) );
                                                                ImGui::Text( "healthbar lower color" );
                                                                ImGui::ColorEdit4( "##healthbar lower color", &GET_VARIABLE( g_variables.colors_health_bar_lower, c_color ) );
                                                        }
                                                } );

                                        

                                                // Weapon name popup
                                                checkbox( "weapon name", &GET_VARIABLE( g_variables.m_weapon_name, bool ), false, true, { 200, -1 }, []( ) {
                                                        ImGui::Text( "weapon name color" );

                                                        ImGui::ColorEdit4( "##weapon name color", &GET_VARIABLE( g_variables.m_weapon_name_color, c_color ),
                                                                           color_picker_alpha_flags );
                                                } );

                                                // Weapon icon popup
                                                checkbox( "weapon icon", &GET_VARIABLE( g_variables.m_weapon_icon, bool ), false, true, { 200, -1 }, []( ) {
                                                        ImGui::Text( "weapon icon color" );

                                                        ImGui::ColorEdit4( "##weapon icon color", &GET_VARIABLE( g_variables.m_weapon_icon_color, c_color ),
                                                                           color_picker_alpha_flags );
                                                } );

                                                // Weapon ammo bar popup
                                                checkbox( "weapon ammo bar", &GET_VARIABLE( g_variables.m_player_ammo_bar, bool ), false, true, { 200, -1 }, []( ) {
                                                        ImGui::Text( "weapon ammo bar color" );

                                                        ImGui::ColorEdit4( "##weapon ammo bar color", &GET_VARIABLE( g_variables.m_player_ammo_bar_color, c_color ),
                                                                           color_picker_alpha_flags );
                                                } );

                                                // Player skeleton popup
                                                checkbox( "player skeleton", &GET_VARIABLE( g_variables.m_players_skeleton, bool ), false, true, { 200, -1 }, []( ) {
                                                        const char* skeleton_types[] = { "normal", "lag compensated" };
                                                        combo( "skeleton type", GET_VARIABLE( g_variables.m_players_skeleton_type, int ), skeleton_types, 2 );
                                                        ImGui::Spacing( );
                                                        ImGui::Text( "player skeleton color" );
                                                        ImGui::ColorEdit4( "##player skeleton color", &GET_VARIABLE( g_variables.m_players_skeleton_color, c_color ),
                                                                           color_picker_alpha_flags );
                                                } );

                                                // Player avatar (без popup)
                                                checkbox( "players avatar", &GET_VARIABLE( g_variables.m_players_avatar, bool ) );

                                                // Out of FOV arrows popup
                                                checkbox(
                                                        "out of fov arrows", &GET_VARIABLE( g_variables.m_out_of_fov_arrows, bool ), false, true, ImVec2( 200.f, -1.f ), []( ) {
                                                                ImGui::SetCursorPosX( 25.f );
                                                                ImGui::Text( "fov arrows color" );
                                                                ImGui::ColorEdit4( "##out of fov arrows color", &GET_VARIABLE( g_variables.m_out_of_fov_arrows_color, c_color ),
                                                                               color_picker_alpha_flags );
                                                                ImGui::Spacing( );
                                                                slider_float( "arrows size", &GET_VARIABLE( g_variables.m_out_of_fov_arrows_size, float ), 0.1f, 50.f, "%.1f px" );
                                                                slider_int( "arrows distance", &GET_VARIABLE( g_variables.m_out_of_fov_arrows_distance, int ), 10, 500, "%d px" );
                                                        } );
                                                slider_int( "visible alpha", &GET_VARIABLE( g_variables.m_visible_alpha, int ), 0, 255, "%d" );
                                                slider_int( "in visible alpha", &GET_VARIABLE( g_variables.m_invisuals_alpha, int ), 0, 255, "%d" );

                                        }
                                } );

                                // Chams settings
                                ImGui::SetCursorPos( ImVec2( 150.0f + 240.f, 10.0f ) );
                                child( "chams", ImVec2( 230.0f, 310.0f ), []( ) {
                                        // visible chams
                                        checkbox( "visible chams", &GET_VARIABLE( g_variables.m_player_visible_chams, bool ), false, true, { 200.f, -1.f }, []( ) {
                                                // Сначала выбор материала, затем цвет.
                                                const char* chams_materialss[] = { "default", "flat", "glow", "wireframe", "metallic" };

                                                combo( "visible chams material", GET_VARIABLE( g_variables.m_player_visible_chams_material, int ), chams_materialss, 5 );
                                                ImGui::Spacing( );
                                                ImGui::Text( "visible chams color" );
                                                ImGui::ColorEdit4( "##visible chams color", &GET_VARIABLE( g_variables.m_player_visible_chams_color, c_color ),
                                                                   color_picker_alpha_flags );
                                        } );

                                        // invisible chams
                                        checkbox( "invisible chams", &GET_VARIABLE( g_variables.m_player_invisible_chams, bool ), false, true, { 200.f, -1.f }, []( ) {
                                                const char* chams_materialss[] = { "default", "flat", "glow", "wireframe", "metallic" };

                                                combo( "invisible chams material", GET_VARIABLE( g_variables.m_player_invisible_chams_material, int ), chams_materialss, 5 );
                                                ImGui::Spacing( );
                                                ImGui::Text( "invisible chams color" );
                                                ImGui::ColorEdit4( "##invisible chams color", &GET_VARIABLE( g_variables.m_player_invisible_chams_color, c_color ),
                                                                   color_picker_alpha_flags );
                                        } );

                                        // lag compensated chams
                                        checkbox( "lag compensated chams", &GET_VARIABLE( g_variables.m_player_lag_chams, bool ), false, true, { 200.f, -1.f }, []( ) {
                                                const char* chams_materialss[] = { "default", "flat", "glow", "wireframe", "metallic" };

                                                checkbox( "lag compensated chams xqz", &GET_VARIABLE( g_variables.m_player_lag_chams_xqz, bool ) );
                                                combo( "lag compensated chams material", GET_VARIABLE( g_variables.m_player_lag_chams_material, int ), chams_materialss, 5 );
                                                const char* lag_chams_types[] = { "oldest record", "all records" };
                                        
                                                combo( "lag compensated chams type", GET_VARIABLE( g_variables.m_player_lag_chams_type, int ), lag_chams_types, 2 );
                                                ImGui::Spacing( );
                                                ImGui::Text( "lagcomp chams color" );
                                                ImGui::ColorEdit4( "##lagcomp chams color", &GET_VARIABLE( g_variables.m_player_lag_chams_color, c_color ),
                                                                   color_picker_alpha_flags );
                                        } );
                                } );

                                // Glow settings
                                ImGui::SetCursorPos( ImVec2( 150.0f, 10.0f + 380.f ) );
                                child( "glow", ImVec2( 230.0f, 60.0f ), []( ) {
                                        // Перемещаем цвета в popup чекбокса "player glow"
                                        checkbox( "player glow", &GET_VARIABLE( g_variables.m_glow_enable, bool ), false, true, { 200.f, -1.f }, []( ) {
                                                ImGui::Text( "player visible glow color" );

                                                ImGui::ColorEdit4( "##player vis glow color", &GET_VARIABLE( g_variables.m_glow_vis_color, c_color ),
                                                                   color_picker_alpha_flags );
                                                ImGui::Spacing( );
                                                ImGui::Text( "player invisible glow color" );
                                                ImGui::ColorEdit4( "##player invis glow color", &GET_VARIABLE( g_variables.m_glow_invis_color, c_color ),
                                                                   color_picker_alpha_flags );
                                        } );
                                } );
                        } 
else {
                                ImGui::SetCursorPos( ImVec2( 150.0f, 10.0f ) );
                                child( "world", ImVec2( 230.0f, 410.0f ), []( ) {
                                        checkbox( "precipitation", &GET_VARIABLE( g_variables.m_precipitation, bool ), false, true, { 200, -1 }, []( ) {
                                                const char* precipitation_types[] = { "rain", "ash", "rain storm", "snow" };
                                                combo( "type", GET_VARIABLE( g_variables.m_precipitation_type, int ), precipitation_types, 4 );
                                        } );


                                                checkbox( "disable post processing", &GET_VARIABLE( g_variables.m_disable_post_processing, bool ) );
                                        checkbox( "remove panorama blur", &GET_VARIABLE( g_variables.m_remove_panorama_blur, bool ) );
#ifdef _DEBUG
                                        checkbox( "disable interp", &GET_VARIABLE( g_variables.m_disable_interp, bool ) );
#endif

                                        checkbox( "fog", &GET_VARIABLE( g_variables.m_fog, bool ), false, true, { 200, -1 }, []( ) {
                                                ImGui::Text( "fog color" );
                                                ImGui::ColorEdit4( "##fog color picker", &GET_VARIABLE( g_variables.m_fog_color, c_color ), color_picker_alpha_flags );
                                                slider_float( "start ", &GET_VARIABLE( g_variables.m_fog_start, float ), 0.f, 5000.f, "%.0f u" );
                                                slider_float( "end ", &GET_VARIABLE( g_variables.m_fog_end, float ), 0.f, 5000.f, "%.0f u" );
                                        } );

                                        checkbox( "sunset", &GET_VARIABLE( g_variables.m_sun_set, bool ), false, true, { 200, -1 }, []( ) {
                                                if ( ImGui::IsItemHovered( ) ) {
                                                        ImGui::SetTooltip( "if you want restore shadows setup all on zero" );
                                                }
                                                slider_float( "sunset rot x", &GET_VARIABLE( g_variables.m_sun_set_rot_x, float ), -1000.f, 1000.f, "%.2f" );
                                                slider_float( "sunset rot y", &GET_VARIABLE( g_variables.m_sun_set_rot_y, float ), -1000.f, 1000.f, "%.2f" );
                                                slider_float( "sunset rot z", &GET_VARIABLE( g_variables.m_sun_set_rot_z, float ), -1000.f, 1000.f, "%.2f" );
                                                slider_float( "sunset rotation speed", &GET_VARIABLE( g_variables.m_sun_set_rotation_speed, float ), 0.0f, 1.0f, "%.2f" );
                                        } );

                                        checkbox( "custom smoke color", &GET_VARIABLE( g_variables.m_custom_smoke, bool ), false, true, { 200, -1 }, []( ) {
                                                ImGui::Text( "custom smoke color" );
                                                ImGui::ColorEdit4( "##custom smoke color picker", &GET_VARIABLE( g_variables.m_custom_smoke_color, c_color ),
                                                                   color_picker_alpha_flags );
                                        } );

                                        checkbox( "custom molotov color", &GET_VARIABLE( g_variables.m_custom_molotov, bool ), false, true, { 200, -1 }, []( ) {
                                                ImGui::Text( "custom molotov color" );
                                                ImGui::ColorEdit4( "##custom molotov color picker", &GET_VARIABLE( g_variables.m_custom_molotov_color, c_color ),
                                                                   color_picker_alpha_flags );
                                        } );

                                        checkbox( "custom blood color", &GET_VARIABLE( g_variables.m_custom_blood, bool ), false, true, { 200, -1 }, []( ) {
                                                ImGui::Text( "custom blood color" );
                                                ImGui::ColorEdit4( "##custom blood color picker", &GET_VARIABLE( g_variables.m_custom_blood_color, c_color ),
                                                                   color_picker_alpha_flags );
                                        } );

                                        checkbox( "custom precipitation color", &GET_VARIABLE( g_variables.m_custom_precipitation, bool ), false, true, { 200, -1 },
                                                  []( ) {
                                                                  ImGui::Text( "custom precipitation color" );
                                                                  ImGui::ColorEdit4( "##custom precipitation color picker",
                                                                             &GET_VARIABLE( g_variables.m_custom_precipitation_color, c_color ), color_picker_alpha_flags );
                                                          } );

                                        checkbox( "motion blur", &GET_VARIABLE( g_variables.m_motion_blur, bool ), false, true, { 200, -1 }, []( ) {
                                                
                                                
                                                                checkbox( "forward enabled", &GET_VARIABLE( g_variables.m_motion_forward_move_blur, bool ) );
                                                                slider_float( "strength", &GET_VARIABLE( g_variables.m_motion_strength, float ), 0.0f, 10.0f, "%.2f" );
                                                                slider_float( "falling intensity", &GET_VARIABLE( g_variables.m_motion_falling_intensity, float ), 0.0f, 20.0f,
                                                                                "%.2f" );
                                                                slider_float( "rotation intensity", &GET_VARIABLE( g_variables.m_motion_rotate_intensity, float ), 0.0f, 20.0f,
                                                                                "%.2f" );
                                                                slider_float( "falling min", &GET_VARIABLE( g_variables.m_motion_falling_minimum, float ), 0.0f, 50.0f,
                                                                                "%.2f" );
                                                                slider_float( "falling max", &GET_VARIABLE( g_variables.m_motion_falling_maximum, float ), 0.0f, 50.0f,
                                                                                "%.2f" );
                                                        
                                        } );

                                        combo( "skybox", GET_VARIABLE( g_variables.m_sky_box, int ), (const char**)skyboxList.data( ), skyboxList.size( ) );
                                        if ( GET_VARIABLE( g_variables.m_sky_box, int ) == 26 ) {
                                                static char buf[ 256 ];
                                                ImGui::Spacing( );
                                                ImGui::Spacing( );
                                                ImGui::Spacing( );
                                                ImGui::InputText( "skybox filename", buf, sizeof( buf ) );
                                                g_variables.m_custom_sky_box = buf;
                                                if ( ImGui::IsItemHovered( ) )
                                                        ImGui::SetTooltip( "skybox files must be put in csgo/materials/skybox/ " );
                                        }

                                        ImGui::Spacing( );
                                        ImGui::SetCursorPosX( 10.f );
                                        ImGui::Text( "accent color" );

                                        ImGui::ColorEdit4( "##accent color", &GET_VARIABLE( g_variables.m_accent, c_color ), color_picker_no_alpha_flags );
                        
                                }
                                                );

                        }
                }


                // Вкладка "Misc" (tab_number == 2)
                if ( tab_number == 2 ) {
                        if ( subtab_number == 0 ) { // Под вкладка "general"
                                ImGui::SetCursorPos( ImVec2( 150.0f, 10.0f ) );
                                child( "game", ImVec2( 230.0f, 310.0f ), []( ) {
                                        checkbox( "spectator list", &GET_VARIABLE( g_variables.m_spectators_list, bool ), false, true, ImVec2( 200.f, -1.f ), []( ) {
                                                ImGui::SetCursorPosX( 25.f );
                                                ImGui::Text( "spectating local color" );
                                                ImGui::ColorEdit4( "##spectator list text color one", &GET_VARIABLE( g_variables.m_spectators_list_text_color_one, c_color ),
                                                                   color_picker_alpha_flags );
                                                ImGui::SetCursorPosX( 25.f );
                                                ImGui::Text( "spectating other color" );
                                                ImGui::ColorEdit4( "##spectator list text color two", &GET_VARIABLE( g_variables.m_spectators_list_text_color_two, c_color ),
                                                                   color_picker_alpha_flags );
                                                const char* listed_players[] = { "all spectators", "local spectators" };
                                                combo( "listed players", GET_VARIABLE( g_variables.m_spectators_list_type, int ), listed_players, 2 );
                                                ImGui::SetCursorPosX( 25.f );
                                                ImGui::Text( "line color" );
                                                ImGui::ColorEdit4( "##spectator list line color", &GET_VARIABLE( g_variables.m_spectators_list_line_color, c_color ),
                                                                   color_picker_alpha_flags );
                                                checkbox( "spectator avatars", &GET_VARIABLE( g_variables.m_spectators_avatar, bool ) );
                                        } );

                                        checkbox( "sniper crosshair", &GET_VARIABLE( g_variables.m_sniper_crosshair, bool ) );
                                        checkbox( "practice window", &GET_VARIABLE( g_variables.m_practice_window, bool ), false, true, ImVec2( 200, -1 ), []( ) {
                                                ImGui::Text( "practice checkpoint key" );
                                                keybind( "practice cp key", &GET_VARIABLE( g_variables.m_practice_cp_key, key_bind_t ) );
                                                ImGui::Text( "practice teleport key" );
                                                keybind( "practice tp key", &GET_VARIABLE( g_variables.m_practice_tp_key, key_bind_t ) );
                                        } );
                                        const char* displayed_logs[] = { "damage", "team damage", "purchase", "votes" };
                                        multi_combo( "displayed logs", GET_VARIABLE( g_variables.m_log_types, std::vector< bool > ), displayed_logs,
                                                     GET_VARIABLE( g_variables.m_log_types, std::vector< bool > ).size( ) );
                                        checkbox( "hit sound", &GET_VARIABLE( g_variables.m_hit_sound, bool ) );
                                        checkbox( "hit marker", &GET_VARIABLE( g_variables.m_hit_marker, bool ) );
                                        checkbox( "scaleform", &GET_VARIABLE( g_variables.m_scaleform, bool ) );
                                        checkbox( "watermark", &GET_VARIABLE( g_variables.m_watermark, bool ), false, true, { 200, -1 },
                                                  []( ) {
                                                        const char* wm_styles[ ] = { "classic", "minimal", "gradient", "rainbow", "glow pulse", "neon", "accent bar" };
                                                        combo( "watermark style", GET_VARIABLE( g_variables.m_watermark_style, int ), wm_styles, 7 );

                                                        static char wm_text_buf[ 48 ] = { };

                                                        ImGui::InputText( "##watermark text", wm_text_buf, sizeof( wm_text_buf ) );

                                                        if ( ImGui::IsItemEdited( ) || ImGui::IsItemDeactivatedAfterEdit( ) )
                                                                GET_VARIABLE( g_variables.m_watermark_text, std::string ) = wm_text_buf;

                                                        // keep the text box in sync with the config while it is not being edited
                                                        if ( !ImGui::IsItemActive( ) ) {
                                                                const auto& wm_text = GET_VARIABLE( g_variables.m_watermark_text, std::string );
                                                                if ( wm_text != wm_text_buf )
                                                                        strncpy_s( wm_text_buf, wm_text.c_str( ), _TRUNCATE );
                                                        }

                                                        if ( ImGui::IsItemHovered( ) )
                                                                ImGui::SetTooltip( "text shown in the watermark ( top right corner )" );
                                                  } );
                                        checkbox( "media player", &GET_VARIABLE( g_variables.trackdispay, bool ) );
                                        if ( button( "force hud update" ) )
                                                g_ctx.force_full_update = true;
                                        ImGui::SetCursorPosX( 25.f );
                                        ImGui::Text( "accent color ( menu theme )" );
                                        ImGui::ColorEdit4( "##accent color", &GET_VARIABLE( g_variables.m_accent, c_color ),
                                                           color_picker_alpha_flags );
                                        ImGui::SetCursorPosX( 25.f );
                                        ImGui::Text( "accent color ( menu theme )" );
                                        ImGui::ColorEdit4( "##accent color", &GET_VARIABLE( g_variables.m_accent, c_color ),
                                                           color_picker_alpha_flags );
                                        ImGui::SetCursorPosX( 25.f );
                                        ImGui::Text( "accent color ( menu theme )" );
                                        ImGui::ColorEdit4( "##accent color", &GET_VARIABLE( g_variables.m_accent, c_color ),
                                                           color_picker_alpha_flags );
#ifdef _DEBUG
                                        static int ctx_panel = 0;
                                        static char buffer[ 1024 * 16 ];
                                        input_text( "panorama script text", buffer, buffer, IM_ARRAYSIZE( buffer ), -1, 20, NULL );
                                        const char* ctx_panel_items[] = { "CSGOHud", "CSGOMainMenu" };
                                        combo( "context panel", ctx_panel, ctx_panel_items, 1 );
                                        if ( ImGui::Button( "run panorama script" ) )
                                                g_scaleform.m_uiengine->run_script( ctx_panel == 0 ? g_scaleform.m_hud_panel : g_scaleform.m_menu_panel, buffer,
                                                                                    ctx_panel == 0 ? "panorama/layout/hud/hud.xml" : "panorama/layout/mainmenu.xml", 8, 10,
                                                                                    false, false );
#endif
                                        
                                } );
                        
                                
                        }
                }

                // Вкладка "Movement" (tab_number == 3)
                if (tab_number == 3)
                {
                        ImGui::SetCursorPos( ImVec2( 150.0f, 10.0f ) );
                        child( "main", ImVec2( 230.0f, 450.0f ), []( ) { 
                                checkbox( "bunny hop", &GET_VARIABLE( g_variables.m_bunny_hop, bool ) );
                                checkbox( "edge jump", &GET_VARIABLE( g_variables.m_edge_jump, bool ), false, true, { 200, -1 }, [](){
                                        
                                        checkbox( "on ladders", &GET_VARIABLE( g_variables.m_edge_jump_ladder, bool ) );
                                        ImGui::Text( "edge jump key" );
                                        keybind( "edge jump key ", &GET_VARIABLE( g_variables.m_edge_jump_key, key_bind_t ) );
                                        } );
                                
                                
                                checkbox( "long jump", &GET_VARIABLE( g_variables.m_long_jump, bool ), false, true, { 200, -1 },[](){
                                        ImGui::Text( "long jump key" );
                                        keybind( "long jump key ", &GET_VARIABLE( g_variables.m_long_jump_key, key_bind_t ) );
                                        } );
                                
                                checkbox( "fire man", &GET_VARIABLE( g_variables.m_fire_man, bool ), false, true, { 200, -1 },
                                          []( ) 
                                        {
                                        ImGui::Text( "fire man key" );
                                        keybind( "fire man key ", &GET_VARIABLE( g_variables.m_fire_man_key, key_bind_t ) );
                                        } );
                                
                                checkbox( "mini jump", &GET_VARIABLE( g_variables.m_mini_jump, bool ), false, true, { 200, -1 },
                                          []( ) { 
                                                
                                                ImGui::Text( "mini jump key" );
                                                keybind( "mini jump key ", &GET_VARIABLE( g_variables.m_mini_jump_key, key_bind_t ) );
                                                checkbox( "hold crouch", &GET_VARIABLE( g_variables.m_mini_jump_hold_duck, bool ) );
                                        
                                        
                                        } );
                        
                                
                                
                                checkbox( "jump bug", &GET_VARIABLE( g_variables.m_jump_bug, bool ), false, true, { 200, -1 }, [](){ 
                                        ImGui::Text( "jump bug key" );
                                        keybind( "jump bug key ", &GET_VARIABLE( g_variables.m_jump_bug_key, key_bind_t ) );
                                        } );

                                checkbox( "texture bug", &GET_VARIABLE( g_variables.m_texture_bug, bool ), false, true, { 200, -1 }, []() {
                                        ImGui::Text( "texture bug key" );
                                        keybind( "texture bug key", &GET_VARIABLE( g_variables.m_texture_bug_key, key_bind_t ) );
                                        slider_float( "reach", &GET_VARIABLE( g_variables.m_texture_bug_reach, float ), 0.f, 48.f, "%.1f u" );
                                        static const char* tb_types[] = { "ducking", "standing" };
                                        multi_combo( "types allowed", GET_VARIABLE( g_variables.m_texture_bug_types, std::vector< bool > ),
                                                     tb_types, GET_VARIABLE( g_variables.m_texture_bug_types, std::vector< bool > ).size( ) );
                                        checkbox( "wallstrafe", &GET_VARIABLE( g_variables.m_texture_bug_wallstrafe, bool ) );
                                        } );
                        
                                checkbox("edge bug", &GET_VARIABLE(g_variables.edge_bug, bool), false, true, { 200, -1 }, []() {
                                        
                                        ImGui::Text( "edge bug key" );
                                        keybind( "edge bug key", &GET_VARIABLE( g_variables.edge_bug_key, key_bind_t ) );
                                        checkbox( "advanced search", &GET_VARIABLE( g_variables.EdgeBugAdvanceSearch, bool ) );
                                        checkbox( "silent edgebug", &GET_VARIABLE( g_variables.SiletEdgeBug, bool ) );
                                        checkbox( "extra advanced search", &GET_VARIABLE( g_variables.MegaEdgeBug, bool ) );
                                        if ( GET_VARIABLE( g_variables.EdgeBugAdvanceSearch, bool ) ) {
                                                checkbox( "autostrafe to edge", &GET_VARIABLE( g_variables.AutoStrafeEdgeBug, bool ) );
                                                if ( !GET_VARIABLE( g_variables.AutoStrafeEdgeBug, bool ) ) {
                                                        slider_float( "strafe angle", &GET_VARIABLE( g_variables.deltaScaler, float ), 1.f, 179.f, "%.0f" );
                                                        const char* delta_types[] = { "full", "two-fifths", "half", "quarter" };
                                                        combo( "delta scale", GET_VARIABLE( g_variables.DeltaType, int ), delta_types,4 );
                                                }
                                        }

                                        slider_int( "edgebug ticks", &GET_VARIABLE( g_variables.EdgeBugTicks, int ), 1, 64, "%d" );
                                        slider_float( "edgebug lock", &GET_VARIABLE( g_variables.EdgeBugMouseLock, float ), 0.0f, 10.0f, "%.2f" );

                                        checkbox( "edgebug sound", &GET_VARIABLE( g_variables.EdgeBugSound, bool ) );
                                        if ( GET_VARIABLE( g_variables.EdgeBugSound, bool ) ) {
                                                static char eb_sound_path_buf[ 260 ] = { };

                                                ImGui::InputText( "##edgebug sound path", eb_sound_path_buf, sizeof( eb_sound_path_buf ) );

                                                if ( ImGui::IsItemEdited( ) || ImGui::IsItemDeactivatedAfterEdit( ) )
                                                        GET_VARIABLE( g_variables.EdgeBugSoundPath, std::string ) = eb_sound_path_buf;

                                                // keep the text box in sync with the config value while it is not being edited
                                                if ( !ImGui::IsItemActive( ) ) {
                                                        const auto& eb_sound_path = GET_VARIABLE( g_variables.EdgeBugSoundPath, std::string );
                                                        if ( eb_sound_path != eb_sound_path_buf )
                                                                strncpy_s( eb_sound_path_buf, eb_sound_path.c_str( ), _TRUNCATE );
                                                }

                                                if ( ImGui::IsItemHovered( ) )
                                                        ImGui::SetTooltip( "folder with edgebug.wav or full path to a .wav file ( e.g. C:\\lobotomy\\sounds )" );

                                                if ( button( "play test sound" ) )
                                                        g_edgebug.play_sound( );
                                        }
                        

                                        } );
                                
                                checkbox( "pixel surf", &GET_VARIABLE( g_variables.m_pixel_surf, bool ), false, true, { 200, -1 },
                                          []( ) { 
                                                ImGui::Text( "pixel surf key" );        
                                                keybind( "pixel surf key ", &GET_VARIABLE( g_variables.m_pixel_surf_key, key_bind_t ) );
                        
                                                checkbox( "pixel surf fix", &GET_VARIABLE( g_variables.m_pixel_surf_fix, bool ) ); } );
                                
                                checkbox( "pixelsurf assistant", &GET_VARIABLE( g_variables.m_pixel_surf_assist, bool ), false, true, { 200, -1 }, []( ) { 
                                                ImGui::Text( "pixelsurf assist" );
                                        checkbox( "broke hop", &GET_VARIABLE( g_variables.m_pixel_surf_assist_brokehop, bool ) );
                                        checkbox( "render pixelsurf assist", &GET_VARIABLE( g_variables.m_pixel_surf_assist_render, bool ) );
                                        slider_float( "unlock factor", &GET_VARIABLE( g_variables.m_pixel_surf_assist_lockfactor, float ), 0.f, 1.f, "%.0f" );
                                        slider_int( "predicting ticks", &GET_VARIABLE( g_variables.m_pixel_surf_assist_ticks, int ), 12, 64, "%d" );
                                        slider_float( "pixelsurf assistant radius", &GET_VARIABLE( g_variables.m_pixel_surf_assist_radius, float ), 40.f, 300.f, "%.0f" );

                                        ImGui::Text( "pixelsurf assistant: " );
                                        keybind( "pixelsurf assistant key", &GET_VARIABLE( g_variables.m_pixel_surf_assist_key, key_bind_t ) );
                                        ImGui::Text( "pixelsurf assistant point: " );
                                        keybind( "pixelsurf assistant point key", &GET_VARIABLE( g_variables.m_pixel_surf_assist_point_key, key_bind_t ) );
                                        ImGui::Text( "pixel finder" );
                                        keybind( "pixel finder key", &GET_VARIABLE( g_variables.m_pixel_finder_key, key_bind_t ) );
                                        const char* pixel_surf_assist_types[] = { "standard", "advanced" };
                                        combo( "type", GET_VARIABLE( g_variables.m_pixel_surf_assist_type, int ), pixel_surf_assist_types,2 );
                                        if ( GET_VARIABLE( g_variables.m_pixel_surf_assist_type, int ) > 0 ) {
                                                slider_float( "delta scale", &GET_VARIABLE( g_variables.m_pixelsurf_assist_delta_scale, float ), 0.f, 1.f, "%.0f" );
                                        }
                                        
                                        } );

                                checkbox( "pixelsurf assistant point", &GET_VARIABLE( g_variables.m_pixel_surf_assist, bool ), false, true, { 200, -1 },
                                        []( ) { 
                                                ImGui::Text( "pixelsurf assistant point key" );
                                                keybind( "pixelsurf assistant point key", &GET_VARIABLE( g_variables.m_pixel_jump_point_key, key_bind_t ) );
                                        } );
                                
                                checkbox("bounce assistant", &GET_VARIABLE(g_variables.m_bouncee_assist, bool), false, true, { 200, -1 }, []() {
                                        ImGui::Text( "bounce assist" );
                                        checkbox( "broke hop", &GET_VARIABLE( g_variables.m_bouncee_assist_brokehop, bool ) );
                                        checkbox( "render bounce assist", &GET_VARIABLE( g_variables.m_bouncee_assist_render, bool ) );
                                        ImGui::Text( "bounce assistant: " );
                                        keybind( "bounce assistant key", &GET_VARIABLE( g_variables.m_bounce_assist_key, key_bind_t ) );
                                        ImGui::Text( "bounce assistant point: " );
                                        keybind( "bounce assistant point key", &GET_VARIABLE( g_variables.m_bounce_assist_point_key, key_bind_t ) );
                                        slider_float( "bounce assistant radius", &GET_VARIABLE( g_variables.m_bouncee_assist_radius, float ), 40.f, 300.f,  "%.0f" );
                        

                                        } );
                                


                                checkbox( "auto bounce", &GET_VARIABLE( g_variables.m_auto_bounce, bool ), false, true, { 200, -1 }, []() {
                                        ImGui::Text( "auto bounce key" );
                                        keybind( "auto bounce key", &GET_VARIABLE( g_variables.m_auto_bounce_key, key_bind_t ) );
                                        static const char* ab_modes[] = { "drop in", "keep speed", "stop" };
                                        combo( "mode", GET_VARIABLE( g_variables.m_auto_bounce_mode, int ), ab_modes, 3 );
                                        slider_float( "min speed gain", &GET_VARIABLE( g_variables.m_auto_bounce_min_gain, float ), 0.f, 500.f, "%.0f" );
                                        slider_float( "max sim reach", &GET_VARIABLE( g_variables.m_auto_bounce_reach, float ), 16.f, 512.f, "%.0f u" );
                                        checkbox( "auto jump", &GET_VARIABLE( g_variables.m_auto_bounce_jump, bool ) );
                                        checkbox( "crouch jump", &GET_VARIABLE( g_variables.m_auto_bounce_crouch, bool ) );
                                        checkbox( "auto align", &GET_VARIABLE( g_variables.m_auto_bounce_align, bool ) );
                                        checkbox( "visualize", &GET_VARIABLE( g_variables.m_auto_bounce_visualize, bool ) );
                                        if ( GET_VARIABLE( g_variables.m_auto_bounce_visualize, bool ) ) {
                                                ImGui::Text( "visualize color" );
                                                ImGui::ColorEdit4( "##auto bounce viz color", &GET_VARIABLE( g_variables.m_auto_bounce_visualize_color, c_color ),
                                                                   color_picker_alpha_flags );
                                        }

                                        checkbox( "bounce sound", &GET_VARIABLE( g_variables.AutoBounceSound, bool ) );
                                        if ( GET_VARIABLE( g_variables.AutoBounceSound, bool ) ) {
                                                static char ab_sound_path_buf[ 260 ] = { };

                                                ImGui::InputText( "##auto bounce sound path", ab_sound_path_buf, sizeof( ab_sound_path_buf ) );

                                                if ( ImGui::IsItemEdited( ) || ImGui::IsItemDeactivatedAfterEdit( ) )
                                                        GET_VARIABLE( g_variables.AutoBounceSoundPath, std::string ) = ab_sound_path_buf;

                                                // keep the text box in sync with the config value while it is not being edited
                                                if ( !ImGui::IsItemActive( ) ) {
                                                        const auto& ab_sound_path = GET_VARIABLE( g_variables.AutoBounceSoundPath, std::string );
                                                        if ( ab_sound_path != ab_sound_path_buf )
                                                                strncpy_s( ab_sound_path_buf, ab_sound_path.c_str( ), _TRUNCATE );
                                                }

                                                if ( ImGui::IsItemHovered( ) )
                                                        ImGui::SetTooltip( "folder with bounce.wav or full path to a .wav file ( e.g. C:\\lobotomy\\sounds )" );

                                                if ( button( "play test sound" ) )
                                                        g_movement.play_bounce_sound( );
                                        }
                                        } );

                                checkbox( "auto align", &GET_VARIABLE( g_variables.m_auto_align, bool ) );
                                checkbox( "no crouch cooldown", &GET_VARIABLE( g_variables.m_no_crouch_cooldown, bool ) );
                                checkbox( "auto duck", &GET_VARIABLE( g_variables.m_auto_duck, bool ) );
                                checkbox( "ladder bug", &GET_VARIABLE( g_variables.m_ladder_bug, bool ), false, true, { 200, -1 }, []( ) {
                                        ImGui::Text( "ladder bug key" );
                                        keybind( "ladder bug key ", &GET_VARIABLE( g_variables.m_ladder_bug_key, key_bind_t ) );
                                        } );
                                checkbox( "airstuck", &GET_VARIABLE( g_variables.m_air_stuck, bool ), false, true, { 200, -1 }, []( ) { ImGui::Text( "airstuck key" );
                                keybind( "##airstuck key ", &GET_VARIABLE( g_variables.m_air_stuck_key, key_bind_t ) );
                                        ImGui::SliderFloat( "TEST", &g_variables.test_air_stuck, -90.f, 90.f );
                                        } );
                                
                                } );

                        ImGui::SetCursorPos( ImVec2( 150.0f + 240.f, 10.0f ) );
                        child( "indicators", ImVec2( 230.0f, 450.0f ), []( ) {
                                
                                checkbox("velocity indicator", &GET_VARIABLE(g_variables.m_velocity_indicator, bool), false, true, { 200, -1 }, []() {
                                        
                                        checkbox( "show pre speed ", &GET_VARIABLE( g_variables.m_velocity_indicator_show_pre_speed, bool ) );
                                        checkbox( "fade alpha ", &GET_VARIABLE( g_variables.m_velocity_indicator_fade_alpha, bool ) );
                                        checkbox( "custom color   ", &GET_VARIABLE( g_variables.m_velocity_indicator_custom_color, bool ) );
                                        slider_int( "padding  ", &GET_VARIABLE( g_variables.m_velocity_indicator_padding, int ), 5, 100, "%d%%" );
                                        ImGui::Spacing( );
                                        
                                        if ( GET_VARIABLE( g_variables.m_velocity_indicator_custom_color, bool ) ) {
                                                ImGui::Text( "velocity colors start" );
                                                ImGui::ColorEdit4( "##velocity indicator color 1", &GET_VARIABLE( g_variables.m_velocity_indicator_color1, c_color ),
                                                                   color_picker_alpha_flags );
                                        
                                                ImGui::Text( "velocity colors end" );
                                                ImGui::ColorEdit4( "##velocity indicator color 2", &GET_VARIABLE( g_variables.m_velocity_indicator_color2, c_color ),
                                                                   color_picker_alpha_flags );
                                        } else {
                                                ImGui::Text( "velocity colors up" );
                                                ImGui::ColorEdit4( "##velocity indicator color 3", &GET_VARIABLE( g_variables.m_velocity_indicator_color3, c_color ),
                                                                   color_picker_alpha_flags );
                                        
                                                ImGui::Text( "velocity colors down" );
                                                ImGui::ColorEdit4( "##velocity indicator color 4", &GET_VARIABLE( g_variables.m_velocity_indicator_color4, c_color ),
                                                                   color_picker_alpha_flags );
                                        
                                                ImGui::Text( "velocity colors equeal" );
                                                ImGui::ColorEdit4( "##velocity indicator color 5", &GET_VARIABLE( g_variables.m_velocity_indicator_color5, c_color ),
                                                                   color_picker_alpha_flags );
                                        }
                                        } );

                                checkbox("stamina indicator", &GET_VARIABLE(g_variables.m_stamina_indicator, bool), false, true, { 200, -1 }, []() {
                                        
                                        checkbox( "show pre speed       ", &GET_VARIABLE( g_variables.m_stamina_indicator_show_pre_speed, bool ) );
                                        checkbox( "fade alpha   ", &GET_VARIABLE( g_variables.m_stamina_indicator_fade_alpha, bool ) );
                                        slider_int( "padding  ", &GET_VARIABLE( g_variables.m_stamina_indicator_padding, int ), 5, 100, "%d%%" );
                                        ImGui::Spacing( );
                                        ImGui::Text( "stamina colors start" );
                                        
                                        ImGui::ColorEdit4( "##stamina indicator color 1", &GET_VARIABLE( g_variables.m_stamina_indicator_color1, c_color ),
                                                           color_picker_alpha_flags );
                                
                                        ImGui::Text( "stamina colors end" );
                                        ImGui::ColorEdit4( "##stamina indicator color 2", &GET_VARIABLE( g_variables.m_stamina_indicator_color2, c_color ),
                                                           color_picker_alpha_flags );
                                        } );
                                
                                checkbox( "keybind indicators", &GET_VARIABLE( g_variables.m_key_indicators_enable, bool ), false, true, { 200, -1 }, [](){
                                        
                                        
                                        slider_int( "position  ", &GET_VARIABLE( g_variables.m_key_indicators_position, int ), 30, g_ctx.m_height, "%d" );
                                        const char* keybind_indicators[] = { "edgebug",         "pixelsurf",      "edgejump", "longjump",  "minijump",
                                                                                 "jumpbug", "pixelsurf assist", "bounce assist", "fireman",  "ladderbug", "ab" };
                                        multi_combo( "displayed keybinds", GET_VARIABLE( g_variables.m_key_indicators, std::vector< bool > ), keybind_indicators,
                                                           11 );
                                        const char* ind_type[] = { "vertical", "horisontal" };
                                        combo( "keybind indicator type", GET_VARIABLE( g_variables.m_key_indicators_type, int ), ind_type, 2 );
                                        ImGui::Spacing( );
                                        ImGui::Text( "keybind colors" );
                                        ImGui::ColorEdit4( "##keybind color", &GET_VARIABLE( g_variables.m_key_color, c_color ), color_picker_alpha_flags );
                                        ImGui::Text( "keybind success colors" );
                                        ImGui::ColorEdit4( "##keybind success color", &GET_VARIABLE( g_variables.m_key_color_success, c_color ),
                                                           color_picker_alpha_flags );
                                        } );
                                

                        } );
                }

                // Вкладка "Inventory" (tab_number == 4) — skinchanger из def: ножи, скины, агенты, перчатки

                if ( tab_number == 4 ) {
                        static bool edit_t = false;

                        static const char* weapon_items[ ] = {
                                "usp-s", "p2000", "glock-18", "p250", "five-seven", "tec-9", "cz75-auto", "dual berettas", "deagle", "r8 revolver",
                                "famas", "galil ar", "m4a4", "m4a1-s", "ak-47", "sg 553", "aug", "ssg 08", "awp", "scar-20",
                                "g3sg1", "sawed-off", "m249", "negev", "mag-7", "xm1014", "nova", "pp-bizon", "mp5-sd", "mp7",
                                "mp9", "mac-10", "p90", "ump-45"
                        };

                        static const char* knife_items[ ] = {
                                "default", "bayonet", "m9 bayonet", "karambit", "bowie", "butterfly", "falchion", "flip", "gut", "huntsman",
                                "shadow daggers", "navaja", "stiletto", "talon", "ursus", "default ct", "default t", "gold knife", "css knife", "outdoor",
                                "canis", "cord", "skeleton"
                        };

                        static const char* sticker_slot_items[ ] = { "slot 1", "slot 2", "slot 3", "slot 4", "slot 5" };

                        static const char* glove_model_items[ ] = { "default", "broken fang", "bloodhound", "sport", "slick",
                                                                                          "leather wrap", "moto", "specialist", "hydra" };

                        const auto copy_to_buffer = []( char* buffer, std::size_t size, const std::string& value ) {
                                const std::size_t len = value.size( ) < size - 1 ? value.size( ) : size - 1;
                                std::memcpy( buffer, value.c_str( ), len );
                                buffer[ len ] = '\0';
                        };

                        // список китов с поиском ( default + все скины из таблицы )
                        const auto skin_selector = []( const char* label, const std::vector< n_skins::paint_kit_entry_t >& kits, int* paint_kit,
                                                       ImGuiTextFilter& filter ) {
                                if ( kits.empty( ) ) {
                                        slider_int( label, paint_kit, 0, 1200, "%d", 205.f );
                                        return;
                                }

                                const char* current = "default";
                                for ( const auto& kit : kits ) {
                                        if ( kit.m_id == *paint_kit ) {
                                                current = kit.m_name.c_str( );
                                                break;
                                        }
                                }

                                ImGui::Text( "%s: %s", label, current );

                                ImGui::PushID( label );

                                filter.Draw( "search##skin filter", 205.f );

                                if ( ImGui::BeginListBox( "##skin list", ImVec2( -1.f, 90.f ) ) ) {
                                        if ( filter.PassFilter( "default" ) ) {
                                                if ( ImGui::Selectable( "default", *paint_kit == 0 ) )
                                                        *paint_kit = 0;
                                        }

                                        static std::vector< int > matches{ };
                                        matches.clear( );

                                        for ( int i = 0; i < static_cast< int >( kits.size( ) ); i++ ) {
                                                if ( filter.PassFilter( kits[ i ].m_name.c_str( ) ) )
                                                        matches.push_back( i );
                                        }

                                        ImGuiListClipper clipper{ };
                                        clipper.Begin( static_cast< int >( matches.size( ) ) );

                                        while ( clipper.Step( ) ) {
                                                for ( int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++ ) {
                                                        const auto& kit = kits[ matches[ row ] ];

                                                        ImGui::PushID( kit.m_id );

                                                        if ( ImGui::Selectable( kit.m_name.c_str( ), kit.m_id == *paint_kit ) )
                                                                *paint_kit = kit.m_id;

                                                        ImGui::PopID( );
                                                }
                                        }

                                        ImGui::EndListBox( );
                                }

                                ImGui::PopID( );
                        };

                        // селектор по имени ( агенты / модели перчаток ) + свои модели из csgo/models/player
                        const auto name_selector = []( const char* label, const char* const* names, int count, int* index, ImGuiTextFilter& filter,
                                                       const std::vector< std::string >* custom = nullptr, std::string* custom_sel = nullptr ) {
                                const bool custom_on = custom_sel && !custom_sel->empty( );
                                ImGui::Text( "%s: %s", label, custom_on ? custom_sel->c_str( ) : ( *index >= 0 && *index < count ) ? names[ *index ] : "?" );

                                ImGui::PushID( label );

                                filter.Draw( "search##name filter", 205.f );

                                if ( ImGui::BeginListBox( "##name list", ImVec2( -1.f, 90.f ) ) ) {
                                        for ( int i = 0; i < count; i++ ) {
                                                if ( !filter.PassFilter( names[ i ] ) )
                                                        continue;

                                                ImGui::PushID( i );

                                                if ( ImGui::Selectable( names[ i ], !custom_on && *index == i ) ) {
                                                        *index = i;
                                                        if ( custom_sel )
                                                                custom_sel->clear( );
                                                }

                                                ImGui::PopID( );
                                        }

                                        if ( custom && custom_sel && !custom->empty( ) ) {
                                                ImGui::TextDisabled( "custom models" );

                                                for ( const std::string& path : *custom ) {
                                                        if ( !filter.PassFilter( path.c_str( ) ) )
                                                                continue;

                                                        if ( ImGui::Selectable( path.c_str( ), custom_on && *custom_sel == path ) )
                                                                *custom_sel = path;
                                                }
                                        }

                                        ImGui::EndListBox( );
                                }

                                ImGui::PopID( );
                        };

                        // 4 палитры в один ряд
                        const auto color_row = []( const char* id, c_color* colors[ 4 ] ) {
                                const ImVec2 next_position = ImGui::GetCursorPos( );

                                for ( int i = 0; i < 4; i++ ) {
                                        ImGui::SameLine( );

                                        const std::string label = std::string( "##" ) + id + " color " + std::to_string( i );

                                        if ( ImGui::ColorEdit4( label.c_str( ), colors[ i ], color_picker_no_alpha_flags ) )
                                                g_skins.m_forcing_update = true;
                                }

                                ImGui::SetCursorPos( next_position );
                        };

                        // подтянуть настоящие цвета кита ( после композита материала )
                        const auto seed_actual_colors = []( int paint_kit, std::array< c_color*, 4 > colors ) {
                                n_skins::paint_colors_t actual{ };
                                if ( !n_skins::get_actual_colors( paint_kit, actual ) )
                                        return;

                                for ( int i = 0; i < 4; i++ )
                                        *colors[ i ] = c_color( static_cast< int >( actual.m_rgb[ i ][ 0 ] ), static_cast< int >( actual.m_rgb[ i ][ 1 ] ),
                                                                static_cast< int >( actual.m_rgb[ i ][ 2 ] ) );
                        };

                        const auto update_button = []( ) {
                                if ( button( "update", ImVec2( 205, 0 ) ) )
                                        g_skins.m_forcing_update = true;
                        };

                        // сброс векторов на дефолт, сохраняя буфер ( frame stage держит ссылки )
                        const auto reset_ints = []( std::uint32_t var, int fill ) {
                                auto& vec = GET_VARIABLE( var, std::vector< int > );
                                vec.assign( vec.size( ), fill );
                        };

                        const auto reset_floats = []( std::uint32_t var, float fill ) {
                                auto& vec = GET_VARIABLE( var, std::vector< float > );
                                vec.assign( vec.size( ), fill );
                        };

                        const auto reset_bools = []( std::uint32_t var ) {
                                auto& vec = GET_VARIABLE( var, std::vector< bool > );
                                vec.assign( vec.size( ), false );
                        };

                        const auto reset_colors = []( std::uint32_t var ) {
                                auto& vec = GET_VARIABLE( var, std::vector< c_color > );
                                vec.assign( vec.size( ), c_color( 255, 255, 255, 255 ) );
                        };

                        const auto reset_strings = []( std::uint32_t var ) {
                                auto& vec = GET_VARIABLE( var, std::vector< std::string > );
                                vec.assign( vec.size( ), std::string( ) );
                        };

                        // переключатель команды + "same skins on both teams" + копирование настроек
                        const auto team_bar = [ & ]( std::uint32_t same_var, const auto& copy_from_other ) {
                                bool& same = GET_VARIABLE( same_var, bool );

                                if ( !same ) {
                                        for ( const bool t : { false, true } ) {
                                                const bool on = edit_t == t;
                                                if ( on )
                                                        ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyleColorVec4( ImGuiCol_::ImGuiCol_Accent ) );
                                                if ( button( t ? "terrorist" : "counter terrorist", ImVec2( 205, 0 ) ) )
                                                        edit_t = t;
                                                if ( on )
                                                        ImGui::PopStyleColor( );
                                        }
                                }

                                checkbox( "same skins on both teams", &same );

                                if ( !same && button( edit_t ? "copy from counter terrorist" : "copy from terrorist", ImVec2( 205, 0 ) ) )
                                        copy_from_other( edit_t );
                        };

                        // GET_VARIABLE( dst ) = GET_VARIABLE( src ): векторы того же размера сохраняют буфер ( frame stage держит ссылки )
                        const auto copy_team = []( auto* type, std::uint32_t ct, std::uint32_t t, bool to_t ) {
                                using T = std::remove_pointer_t< decltype( type ) >;
                                GET_VARIABLE( to_t ? t : ct, T ) = GET_VARIABLE( to_t ? ct : t, T );
                        };

#define TV( name ) ( tside ? g_variables.name##_tside : g_variables.name )
#define COPY_TEAM( name, type ) copy_team( static_cast< type* >( nullptr ), g_variables.name, g_variables.name##_tside, to_t )

                        // --- под-вкладка weapons: скины оружия + нож ---
                        if ( subtab_number == 0 ) {
                                const bool tside = !GET_VARIABLE( g_variables.m_skins_same_weapons, bool ) && edit_t;

                                ImGui::SetCursorPos( ImVec2( 150.0f, 10.0f ) );
                                child( "inventory", ImVec2( 460.0f, 450.0f ), [ & ] {
                                        ImGui::BeginChild( "weapons_col", ImVec2( 224.f, 0.f ), false ); {

                                        team_bar( g_variables.m_skins_same_weapons, [ & ]( bool to_t ) {
                                                COPY_TEAM( m_weapon_skins_enable, bool );
                                                COPY_TEAM( m_weapon_skins_paint_kit, std::vector< int > );
                                                COPY_TEAM( m_weapon_skins_wear, std::vector< float > );
                                                COPY_TEAM( m_weapon_skins_seed, std::vector< int > );
                                                COPY_TEAM( m_weapon_skins_stattrak, std::vector< bool > );
                                                COPY_TEAM( m_weapon_skins_stattrak_kills, std::vector< int > );
                                                COPY_TEAM( m_weapon_skins_custom_name, std::vector< std::string > );
                                                COPY_TEAM( m_weapon_skins_custom_color, std::vector< bool > );
                                                COPY_TEAM( m_weapon_skins_color_1, std::vector< c_color > );
                                                COPY_TEAM( m_weapon_skins_color_2, std::vector< c_color > );
                                                COPY_TEAM( m_weapon_skins_color_3, std::vector< c_color > );
                                                COPY_TEAM( m_weapon_skins_color_4, std::vector< c_color > );
                                                COPY_TEAM( m_weapon_skins_sticker_kit, std::vector< int > );
                                                COPY_TEAM( m_weapon_skins_sticker_wear, std::vector< float > );
                                                COPY_TEAM( m_weapon_skins_sticker_scale, std::vector< float > );
                                                COPY_TEAM( m_weapon_skins_sticker_rotation, std::vector< float > );
                                                g_skins.m_forcing_update = true;
                                        } );

                                        checkbox( "enable weapon skins", &GET_VARIABLE( TV( m_weapon_skins_enable ), bool ) );

                                        combo( "weapon", GET_VARIABLE( g_variables.m_weapon_skins_selected, int ), weapon_items, 34 );

                                        const int sel        = GET_VARIABLE( g_variables.m_weapon_skins_selected, int );
                                        auto& paint_kits     = GET_VARIABLE( TV( m_weapon_skins_paint_kit ), std::vector< int > );
                                        auto& wears          = GET_VARIABLE( TV( m_weapon_skins_wear ), std::vector< float > );
                                        auto& seeds          = GET_VARIABLE( TV( m_weapon_skins_seed ), std::vector< int > );
                                        auto& stattraks      = GET_VARIABLE( TV( m_weapon_skins_stattrak ), std::vector< bool > );
                                        auto& stattrak_kills = GET_VARIABLE( TV( m_weapon_skins_stattrak_kills ), std::vector< int > );
                                        auto& custom_names   = GET_VARIABLE( TV( m_weapon_skins_custom_name ), std::vector< std::string > );

                                        if ( sel >= 0 && sel < static_cast< int >( paint_kits.size( ) ) && sel < static_cast< int >( wears.size( ) ) &&
                                             sel < static_cast< int >( seeds.size( ) ) && sel < static_cast< int >( stattraks.size( ) ) &&
                                             sel < static_cast< int >( stattrak_kills.size( ) ) && sel < static_cast< int >( custom_names.size( ) ) ) {
                                                static ImGuiTextFilter weapon_skin_filter{ };
                                                skin_selector( "paint kit", g_skins.m_parser_skins, &paint_kits[ sel ], weapon_skin_filter );

                                                slider_float( "wear", &wears[ sel ], 0.0001f, 1.f, "%.4f", 205.f );
                                                slider_int( "pattern", &seeds[ sel ], 0, 1023, "%d", 205.f );

                                                bool stattrak = stattraks[ sel ];
                                                checkbox( "stattrak", &stattrak );
                                                stattraks[ sel ] = stattrak;

                                                if ( stattrak )
                                                        slider_int( "kills", &stattrak_kills[ sel ], 0, 99999, "%d", 205.f );

                                                static char weapon_name[ 41 ] = { };
                                                static int last_selected      = -1;

                                                if ( last_selected != sel ) {
                                                        last_selected = sel;
                                                        copy_to_buffer( weapon_name, sizeof( weapon_name ), custom_names[ sel ] );
                                                }

                                                if ( ImGui::InputText( "##weapon name tag", weapon_name, sizeof( weapon_name ) ) )
                                                        custom_names[ sel ] = weapon_name;
                                                else if ( !ImGui::IsItemActive( ) && custom_names[ sel ] != weapon_name )
                                                        copy_to_buffer( weapon_name, sizeof( weapon_name ), custom_names[ sel ] );

                                                ImGui::Text( "name tag" );

                                                auto& custom_colors = GET_VARIABLE( TV( m_weapon_skins_custom_color ), std::vector< bool > );
                                                auto& colors_1      = GET_VARIABLE( TV( m_weapon_skins_color_1 ), std::vector< c_color > );
                                                auto& colors_2      = GET_VARIABLE( TV( m_weapon_skins_color_2 ), std::vector< c_color > );
                                                auto& colors_3      = GET_VARIABLE( TV( m_weapon_skins_color_3 ), std::vector< c_color > );
                                                auto& colors_4      = GET_VARIABLE( TV( m_weapon_skins_color_4 ), std::vector< c_color > );

                                                if ( sel < static_cast< int >( custom_colors.size( ) ) && sel < static_cast< int >( colors_1.size( ) ) &&
                                                     sel < static_cast< int >( colors_2.size( ) ) && sel < static_cast< int >( colors_3.size( ) ) &&
                                                     sel < static_cast< int >( colors_4.size( ) ) ) {
                                                        bool custom_color = custom_colors[ sel ];
                                                        checkbox( "custom color", &custom_color );
                                                        if ( custom_color != custom_colors[ sel ] ) {
                                                                custom_colors[ sel ]     = custom_color;
                                                                g_skins.m_forcing_update = true;

                                                                if ( custom_color )
                                                                        seed_actual_colors( paint_kits[ sel ],
                                                                                                            { &colors_1[ sel ], &colors_2[ sel ], &colors_3[ sel ], &colors_4[ sel ] } );
                                                        }

                                                        if ( custom_color ) {
                                                                c_color* weapon_colors[ 4 ] = { &colors_1[ sel ], &colors_2[ sel ], &colors_3[ sel ], &colors_4[ sel ] };
                                                                color_row( "weapon", weapon_colors );
                                                        }
                                                }

                                                auto& sticker_kits      = GET_VARIABLE( TV( m_weapon_skins_sticker_kit ), std::vector< int > );
                                                auto& sticker_wears     = GET_VARIABLE( TV( m_weapon_skins_sticker_wear ), std::vector< float > );
                                                auto& sticker_scales    = GET_VARIABLE( TV( m_weapon_skins_sticker_scale ), std::vector< float > );
                                                auto& sticker_rotations = GET_VARIABLE( TV( m_weapon_skins_sticker_rotation ), std::vector< float > );

                                                combo( "sticker slot", GET_VARIABLE( g_variables.m_weapon_skins_sticker_slot, int ), sticker_slot_items, 5 );

                                                const int slot    = GET_VARIABLE( g_variables.m_weapon_skins_sticker_slot, int );
                                                const int sticker = sel * 5 + slot;

                                                if ( slot >= 0 && slot < 5 && sticker < static_cast< int >( sticker_kits.size( ) ) &&
                                                     sticker < static_cast< int >( sticker_wears.size( ) ) && sticker < static_cast< int >( sticker_scales.size( ) ) &&
                                                     sticker < static_cast< int >( sticker_rotations.size( ) ) ) {
                                                        static ImGuiTextFilter sticker_filter{ };
                                                        skin_selector( "sticker", g_skins.m_parser_stickers, &sticker_kits[ sticker ], sticker_filter );

                                                        if ( sticker_kits[ sticker ] > 0 ) {
                                                                slider_float( "sticker wear", &sticker_wears[ sticker ], 0.f, 1.f, "%.4f", 205.f );
                                                                slider_float( "sticker scale", &sticker_scales[ sticker ], 0.1f, 5.f, "%.2f", 205.f );
                                                                slider_float( "sticker rotation", &sticker_rotations[ sticker ], 0.f, 360.f, "%.0f", 205.f );
                                                        }
                                                }
                                        }

                                        update_button( );

                                        static bool open_reset_popup = false;
                                        if ( button( "reset skins", ImVec2( 205, 0 ) ) )
                                                open_reset_popup = true;

                                        if ( open_reset_popup ) {
                                                save_popup( "reset all weapon skins", ImVec2( 220.f, -1.f ), [ & ]( ) {
                                                        if ( button( "yes", ImVec2( 96, 0 ) ) ) {
                                                                for ( const std::uint32_t variable : { g_variables.m_weapon_skins_paint_kit, g_variables.m_weapon_skins_paint_kit_tside,
                                                                                                               g_variables.m_weapon_skins_seed, g_variables.m_weapon_skins_seed_tside,
                                                                                                               g_variables.m_weapon_skins_stattrak_kills, g_variables.m_weapon_skins_stattrak_kills_tside,
                                                                                                               g_variables.m_weapon_skins_sticker_kit, g_variables.m_weapon_skins_sticker_kit_tside } )
                                                                        reset_ints( variable, 0 );

                                                                for ( const std::uint32_t variable : { g_variables.m_weapon_skins_wear, g_variables.m_weapon_skins_wear_tside } )
                                                                        reset_floats( variable, 0.0001f );

                                                                for ( const std::uint32_t variable : { g_variables.m_weapon_skins_sticker_wear, g_variables.m_weapon_skins_sticker_wear_tside,
                                                                                                               g_variables.m_weapon_skins_sticker_rotation, g_variables.m_weapon_skins_sticker_rotation_tside } )
                                                                        reset_floats( variable, 0.f );

                                                                for ( const std::uint32_t variable : { g_variables.m_weapon_skins_sticker_scale, g_variables.m_weapon_skins_sticker_scale_tside } )
                                                                        reset_floats( variable, 1.f );

                                                                for ( const std::uint32_t variable : { g_variables.m_weapon_skins_stattrak, g_variables.m_weapon_skins_stattrak_tside,
                                                                                                               g_variables.m_weapon_skins_custom_color, g_variables.m_weapon_skins_custom_color_tside } )
                                                                        reset_bools( variable );

                                                                for ( const std::uint32_t variable : { g_variables.m_weapon_skins_color_1, g_variables.m_weapon_skins_color_1_tside,
                                                                                                               g_variables.m_weapon_skins_color_2, g_variables.m_weapon_skins_color_2_tside,
                                                                                                               g_variables.m_weapon_skins_color_3, g_variables.m_weapon_skins_color_3_tside,
                                                                                                               g_variables.m_weapon_skins_color_4, g_variables.m_weapon_skins_color_4_tside } )
                                                                        reset_colors( variable );

                                                                reset_strings( g_variables.m_weapon_skins_custom_name );
                                                                reset_strings( g_variables.m_weapon_skins_custom_name_tside );

                                                                g_logger.print( "reset all weapon skins" );
                                                                open_reset_popup = false;
                                                        }

                                                        ImGui::SameLine( );

                                                        if ( button( "no", ImVec2( 96, 0 ) ) )
                                                                open_reset_popup = false;
                                                } );
                                        }

                                        } ImGui::EndChild( );

                                        ImGui::SameLine( );

                                        ImGui::BeginChild( "knife_col", ImVec2( 0.f, 0.f ), false ); {

                                        checkbox( "enable knife anims", &GET_VARIABLE( TV( m_knife_anims_enable ), bool ) );
                                        if ( GET_VARIABLE( TV( m_knife_anims_enable ), bool ) ) {
                                                if ( !n_skins::knife_anims_fit( GET_VARIABLE( TV( m_knife_anims_model ), int ),
                                                                                                GET_VARIABLE( TV( m_knife_model ), int ), GET_VARIABLE( TV( m_knife_enable ), bool ) ) )
                                                        ImGui::TextColored( ImVec4( 1.f, 0.4f, 0.4f, 1.f ), "wrong bones for this knife - draw will not play" );

                                                combo( "knife anims", GET_VARIABLE( TV( m_knife_anims_model ), int ), knife_items, 23 );
                                        }

                                        checkbox( "enable knife", &GET_VARIABLE( TV( m_knife_enable ), bool ) );
                                        if ( GET_VARIABLE( TV( m_knife_enable ), bool ) ) {
                                                combo( "knife model", GET_VARIABLE( TV( m_knife_model ), int ), knife_items, 23 );

                                                if ( GET_VARIABLE( TV( m_knife_model ), int ) == 10 )
                                                        checkbox( "fix view", &GET_VARIABLE( TV( m_knife_fix_view ), bool ) );

                                                static ImGuiTextFilter knife_skin_filter{ };
                                                skin_selector( "paint kit", g_skins.m_parser_skins, &GET_VARIABLE( TV( m_knife_paint_kit ), int ), knife_skin_filter );

                                                slider_float( "knife wear", &GET_VARIABLE( TV( m_knife_wear ), float ), 0.0001f, 1.f, "%.4f", 205.f );
                                                slider_int( "knife pattern", &GET_VARIABLE( TV( m_knife_seed ), int ), 0, 1023, "%d", 205.f );

                                                checkbox( "knife stattrak", &GET_VARIABLE( TV( m_knife_stattrak ), bool ) );
                                                if ( GET_VARIABLE( TV( m_knife_stattrak ), bool ) )
                                                        slider_int( "knife kills", &GET_VARIABLE( TV( m_knife_stattrak_kills ), int ), 0, 99999, "%d", 205.f );

                                                static char knife_name[ 41 ] = { };
                                                auto& knife_name_var         = GET_VARIABLE( TV( m_knife_custom_name ), std::string );

                                                if ( ImGui::InputText( "##knife name tag", knife_name, sizeof( knife_name ) ) )
                                                        knife_name_var = knife_name;
                                                else if ( !ImGui::IsItemActive( ) && knife_name_var != knife_name )
                                                        copy_to_buffer( knife_name, sizeof( knife_name ), knife_name_var );

                                                ImGui::Text( "knife name tag" );

                                                bool knife_custom_color = GET_VARIABLE( TV( m_knife_custom_color ), bool );
                                                checkbox( "custom color", &knife_custom_color );
                                                if ( knife_custom_color != GET_VARIABLE( TV( m_knife_custom_color ), bool ) ) {
                                                        GET_VARIABLE( TV( m_knife_custom_color ), bool ) = knife_custom_color;
                                                        g_skins.m_forcing_update                         = true;

                                                        if ( knife_custom_color )
                                                                seed_actual_colors( GET_VARIABLE( TV( m_knife_paint_kit ), int ),
                                                                                            { &GET_VARIABLE( TV( m_knife_color_1 ), c_color ), &GET_VARIABLE( TV( m_knife_color_2 ), c_color ),
                                                                                              &GET_VARIABLE( TV( m_knife_color_3 ), c_color ), &GET_VARIABLE( TV( m_knife_color_4 ), c_color ) } );
                                                }
                                                if ( GET_VARIABLE( TV( m_knife_custom_color ), bool ) ) {
                                                        c_color* knife_colors[ 4 ] = {
                                                                &GET_VARIABLE( TV( m_knife_color_1 ), c_color ), &GET_VARIABLE( TV( m_knife_color_2 ), c_color ),
                                                                &GET_VARIABLE( TV( m_knife_color_3 ), c_color ), &GET_VARIABLE( TV( m_knife_color_4 ), c_color )
                                                        };

                                                        color_row( "knife", knife_colors );
                                                }
                                        }

                                        update_button( );

                                        } ImGui::EndChild( );
                                } );
                        }

                        // --- под-вкладка player: агенты + перчатки ---
                        if ( subtab_number == 1 ) {
                                const bool tside = !GET_VARIABLE( g_variables.m_skins_same_player, bool ) && edit_t;

                                ImGui::SetCursorPos( ImVec2( 150.0f, 10.0f ) );
                                child( "player", ImVec2( 460.0f, 450.0f ), [ & ] {
                                        ImGui::BeginChild( "agents_col", ImVec2( 224.f, 0.f ), false ); {

                                        team_bar( g_variables.m_skins_same_player, [ & ]( bool to_t ) {
                                                COPY_TEAM( m_gloves_enable, bool );
                                                COPY_TEAM( m_gloves_model, int );
                                                COPY_TEAM( m_gloves_paint_kit, int );
                                                COPY_TEAM( m_gloves_wear, float );
                                                COPY_TEAM( m_gloves_seed, int );
                                                copy_team( static_cast< int* >( nullptr ), g_variables.m_agent_ct, g_variables.m_agent_t, to_t );
                                                copy_team( static_cast< std::string* >( nullptr ), g_variables.m_agent_ct_custom, g_variables.m_agent_t_custom, to_t );
                                                g_skins.m_forcing_update = true;
                                        } );

                                        checkbox( "enable agents", &GET_VARIABLE( g_variables.m_agent_enable, bool ) );
                                        if ( GET_VARIABLE( g_variables.m_agent_enable, bool ) ) {
                                                checkbox( "custom models", &GET_VARIABLE( g_variables.m_agent_custom_models, bool ) );
                                                const bool show_custom = GET_VARIABLE( g_variables.m_agent_custom_models, bool );

                                                // is_player_mdl читает весь mdl + vvd: один не закешированный файл за кадр, рескэн при переоткрытии списка
                                                static std::vector< std::string > pending, custom_models;
                                                static std::unordered_map< std::string, bool > is_body;
                                                static int last_frame = -2;
                                                if ( show_custom ) {
                                                        if ( ImGui::GetFrameCount( ) - last_frame > 1 ) {
                                                                pending = g_utilities.game_files( "models/player", ".mdl" );
                                                                std::reverse( pending.begin( ), pending.end( ) );
                                                                custom_models.clear( );
                                                        }
                                                        last_frame = ImGui::GetFrameCount( );

                                                        while ( !pending.empty( ) ) {
                                                                const std::string path = std::move( pending.back( ) );
                                                                pending.pop_back( );
                                                                auto it          = is_body.find( path );
                                                                const bool fresh = it == is_body.end( );
                                                                if ( fresh )
                                                                        it = is_body.emplace( path, g_utilities.is_player_mdl( path ) ).first;
                                                                if ( it->second )
                                                                        custom_models.push_back( path );
                                                                if ( fresh )
                                                                        break;
                                                        }
                                                        if ( !pending.empty( ) )
                                                                ImGui::TextDisabled( "loading custom models... %d left", static_cast< int >( pending.size( ) ) );
                                                }

                                                static ImGuiTextFilter t_agent_filter{ }, ct_agent_filter{ };
                                                if ( GET_VARIABLE( g_variables.m_skins_same_player, bool ) || edit_t )
                                                        name_selector( "t agent", n_skins::AGENT_NAMES, n_skins::AGENT_NAME_COUNT,
                                                                                               &GET_VARIABLE( g_variables.m_agent_t, int ), t_agent_filter,
                                                                                               show_custom ? &custom_models : nullptr, &GET_VARIABLE( g_variables.m_agent_t_custom, std::string ) );
                                                if ( GET_VARIABLE( g_variables.m_skins_same_player, bool ) || !edit_t )
                                                        name_selector( "ct agent", n_skins::AGENT_NAMES, n_skins::AGENT_NAME_COUNT,
                                                                                               &GET_VARIABLE( g_variables.m_agent_ct, int ), ct_agent_filter,
                                                                                               show_custom ? &custom_models : nullptr, &GET_VARIABLE( g_variables.m_agent_ct_custom, std::string ) );
                                        }

                                        update_button( );

                                        } ImGui::EndChild( );

                                        ImGui::SameLine( );

                                        ImGui::BeginChild( "gloves_col", ImVec2( 0.f, 0.f ), false ); {

                                        checkbox( "enable gloves", &GET_VARIABLE( TV( m_gloves_enable ), bool ) );
                                        if ( GET_VARIABLE( TV( m_gloves_enable ), bool ) ) {
                                                static ImGuiTextFilter glove_model_filter{ };
                                                name_selector( "glove model", glove_model_items, 9, &GET_VARIABLE( TV( m_gloves_model ), int ), glove_model_filter );

                                                static ImGuiTextFilter glove_skin_filter{ };
                                                skin_selector( "paint kit", g_skins.m_parser_gloves, &GET_VARIABLE( TV( m_gloves_paint_kit ), int ), glove_skin_filter );

                                                slider_float( "glove wear", &GET_VARIABLE( TV( m_gloves_wear ), float ), 0.0001f, 1.f, "%.4f", 205.f );
                                                slider_int( "glove pattern", &GET_VARIABLE( TV( m_gloves_seed ), int ), 0, 1023, "%d", 205.f );
                                        }

                                        update_button( );

                                        } ImGui::EndChild( );
                                } );
                        }

#undef TV
#undef COPY_TEAM
                }

                // Вкладка "Font" (tab_number == 5)

                if ( tab_number == 5 ) {
                        ImGui::SetCursorPos( ImVec2( 150.0f, 10.0f ) );
                        child( "fonts", ImVec2(230.0f, 410.0f ), []( ) {
                                
                                
                                
                                const char* font_flags[] = { "no hinting",  "no autohint", "force autohint", "light hinting",
                                                                 "monohinting", "bold",        "oblique",        "monochrome" };
                        
                                
                                ImGui::Spacing( );
                                ImGui::Spacing( );
                                ImGui::Spacing( );
                                static ImGuiTextFilter indicator_font_search_filter{ };
                                indicator_font_search_filter.Draw( "search for a font", 210.f );
                                float item_height = ImGui::GetTextLineHeightWithSpacing( );
                                ImVec2 size       = ImVec2( 237, item_height * 5 ); // Ширина 0 означает использование доступной ширины
                                ImGui::SetCursorPosX( -8.f );
                                if ( ImGui::BeginListBox( "##font list", ImVec2( size ) ) ) {
                                        for ( const auto iterator : g_fonts.m_font_file_names ) {
                                                if ( indicator_font_search_filter.PassFilter( iterator.c_str( ) ) )
                                                        if ( ImGui::Selectable( iterator.c_str( ), HASH_RT( GET_VARIABLE( g_variables.m_indicator_font_settings, font_setting_t )
                                                                                                                .m_name.c_str( ) ) == HASH_RT( iterator.c_str( ) ) ) )
                                                                GET_VARIABLE( g_variables.m_indicator_font_settings, font_setting_t ).m_name = iterator;
                                        }
                                        ImGui::EndListBox( );
                                }
                        
                                ImGui::SetCursorPosX( 10.f );
                                if ( button( "reload fonts" ,ImVec2(210,0) ))
                                        g_render.m_reload_fonts = true;
                                ImGui::SetCursorPosX( 10.f );
                                if ( button( "reset to default  ", { 210, 0 } ) ) {
                                        g_render.m_custom_fonts[ e_custom_font_names::custom_font_name_indicator ] =
                                                g_render.m_fonts[ e_font_names::font_name_indicator_29 ];
                                }
                                
                                
                                multi_combo( "font flags ", GET_VARIABLE( g_variables.m_indicator_font_flags, std::vector< bool > ), font_flags,
                                             GET_VARIABLE( g_variables.m_indicator_font_flags, std::vector< bool > ).size( ), ImVec2( 210, 40 ) );
                        

                                slider_int( "size       ", &GET_VARIABLE( g_variables.m_indicator_font_settings, font_setting_t ).m_size, 0, 50, "%d",210.f );

                        } );
                }

                // Вкладка "Config" (tab_number == 6)
                if (tab_number == 6)
                {
                        ImGui::SetCursorPos( ImVec2( 150.0f, 10.0f ) );
                        child( "settings", ImVec2( 230.0f, 410.0f ), []( ) { 
                                ImGui::Spacing( );
                                ImGui::Spacing( );
                                input_text( "config file name", g_menu.m_config_file, g_menu.m_config_file, sizeof( g_menu.m_config_file ),210.f,20.f,NULL );
                                std::string converted_file_name         = g_menu.m_config_file;
                                static std::string selected_config_name = { };
                        
                        
                                float item_height = ImGui::GetTextLineHeightWithSpacing( );
                                ImVec2 size       = ImVec2( 237, item_height * 5 ); // Ширина 0 означает использование доступной ширины
                                ImGui::SetCursorPosX( -8.f );
                                if ( ImGui::BeginListBox( "##config list", size ) ) {
                                        for ( size_t i = 0; i < g_config.m_file_names.size( ); i++ ) {
                                                bool is_selected = ( g_menu.m_selected_config == i );
                                                if ( ImGui::Selectable( g_config.m_file_names[ i ].c_str( ), is_selected ) )
                                                        g_menu.m_selected_config = i;
                                        }
                                        ImGui::EndListBox( );
                                }
                                selected_config_name = !g_config.m_file_names.empty( ) ? g_config.m_file_names[ g_menu.m_selected_config ] : "";
                                ImGui::SetCursorPosX( 10.f );
                                if ( button( "create",ImVec2(210,0)) ) {
                                        if ( !g_config.save( converted_file_name ) )
                                                g_console.print( std::vformat( "failed to create {:s}", std::make_format_args( converted_file_name ) ).c_str( ) );
                                        else
                                                g_logger.print( std::vformat( "created config {:s}", std::make_format_args( converted_file_name ) ).c_str( ) );
                                        converted_file_name.clear( );
                                        g_config.refresh( );
                                }
                                static bool open_save_popup = false;
                                ImGui::SetCursorPosX( 10.f );
                                if ( button( "save", ImVec2( 210, 0 ) ) )
                                        open_save_popup = true;
                                if ( open_save_popup ) {
                                        save_popup( "save confirmation", ImVec2( 220.f, -1.f ), [ & ]( ) {
                                                if ( button( "yes" ) ) {
                                                        saveConfig( "C:\\lobotomy\\points.dat" );
                                                        saveConfig2( "C:\\lobotomy\\bpoints.dat" );
                                                        g_aimbot.save( );
                                                        if ( !g_config.save( selected_config_name ) )
                                                                g_console.print( std::vformat( "failed to save {:s}", std::make_format_args( selected_config_name ) ).c_str( ) );
                                                        else
                                                                g_logger.print( std::vformat( "saved config {:s}", std::make_format_args( selected_config_name ) ).c_str( ) );
                                                        open_save_popup = false;
                                                }
                                                
                                                if ( button( "no" ) ) {
                                                        g_logger.print( std::vformat( "canceled saving config {:s}", std::make_format_args( selected_config_name ) ).c_str( ) );
                                                        open_save_popup = false;
                                                }
                                        } );
                                }
                                ImGui::SetCursorPosX( 10.f );
                                if ( button( "load", ImVec2( 210, 0 ) ) ) {
                                        if ( !g_config.load( selected_config_name ) )
                                                g_console.print( std::vformat( "failed to load {:s}", std::make_format_args( selected_config_name ) ).c_str( ) );
                                        else {
                                                g_logger.print( std::vformat( "loaded config {:s}", std::make_format_args( selected_config_name ) ).c_str( ) );
                                                g_render.m_reload_fonts = true;
                                        }
                                        loadConfig( "C:\\lobotomy\\points.dat" );
                                        loadConfig2( "C:\\lobotomy\\bpoints.dat" );
                                        g_aimbot.load( );
                                }
                                ImGui::SetCursorPosX( 10.f );
                                if ( button( "remove", ImVec2( 210, 0 ) ) ) {
                                        g_logger.print( std::vformat( "removed config {:s}", std::make_format_args( selected_config_name ) ).c_str( ) );
                                        g_config.remove( g_menu.m_selected_config );
                                        g_menu.m_selected_config = 0;
                                }
                                ImGui::SetCursorPosX( 10.f );
                                if ( button( "refresh", ImVec2( 210, 0 ) ) )
                                        g_config.refresh( );
                                ImGui::SetCursorPosX( 10.f );
                                if ( button( "connect to server", ImVec2( 210, 0 ) ) )
                                        g_menu.connect_to_server = true;
                                        
                                
                                
                                } );
                }

                ImGui::PopFont( );
        }
        ImGui::End( );
}
