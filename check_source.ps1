# ============================================================
#  LOBO source checker / fixer v3 (safe version)
#  Run check_source.bat (double click) from your SOURCE folder
#  (the one that has csgo-sdk.sln). Do NOT run it from inside
#  the zip - extract the zip into a folder first!
#  Everything is also saved to check_result.txt (same folder),
#  so nothing is lost even if the window closes.
# ============================================================
$ErrorActionPreference = 'Continue'
try { Set-Location -LiteralPath $PSScriptRoot } catch { }
try { [IO.Directory]::SetCurrentDirectory($PSScriptRoot) } catch { }

$logPath = Join-Path $PSScriptRoot 'check_result.txt'
$script:broken = $false
$script:filesBroken = $false
$script:missList = New-Object System.Collections.Generic.List[string]
$script:wrongList = New-Object System.Collections.Generic.List[string]

try { Remove-Item -LiteralPath $logPath -Force -ErrorAction SilentlyContinue } catch { }

function Write-Log([string]$text, [string]$color = '') {
    if ($color -ne '') { Write-Host $text -ForegroundColor $color } else { Write-Host $text }
    try { Add-Content -LiteralPath $logPath -Value $text -Encoding UTF8 } catch { }
}

function Get-FileSize([string]$rel) {
    $full = Join-Path (Get-Location).Path ($rel.Replace('/', '\'))
    try { return (Get-Item -LiteralPath $full -ErrorAction Stop).Length } catch { }
    if ($full.Length -gt 240) {
        try { return (Get-Item -LiteralPath ('\?\' + $full) -ErrorAction Stop).Length } catch { }
    }
    return -1
}

function Verify-All {
    $script:missList.Clear()
    $script:wrongList.Clear()
    foreach ($k in $expected.Keys) {
        $len = Get-FileSize $k
        if ($len -lt 0) { $script:missList.Add($k) }
        elseif ($len -ne $expected[$k]) { $script:wrongList.Add($k) }
    }
}

function Show-Problems {
    if ($script:missList.Count -gt 0) {
        Write-Log ('[X] MISSING files: ' + $script:missList.Count) 'Red'
        $script:missList | Select-Object -First 25 | ForEach-Object { Write-Log ('    - ' + $_) }
        if ($script:missList.Count -gt 25) { Write-Log ('    ... and ' + ($script:missList.Count - 25) + ' more') }
    }
    if ($script:wrongList.Count -gt 0) {
        Write-Log ('[X] DAMAGED files (wrong size): ' + $script:wrongList.Count) 'Red'
        $script:wrongList | Select-Object -First 25 | ForEach-Object {
            Write-Log ('    - ' + $_ + '   (now ' + (Get-FileSize $_) + ' bytes, must be ' + $expected[$_] + ')')
        }
        if ($script:wrongList.Count -gt 25) { Write-Log ('    ... and ' + ($script:wrongList.Count - 25) + ' more') }
    }
}

function Try-Repair {
    Write-Log ''
    Write-Log '--- Auto-repair from lobo_edgebug_sound.zip ---'
    $needZip = 18669902
    $dirs = @(
        $PSScriptRoot,
        (Split-Path -Parent $PSScriptRoot),
        ([Environment]::GetFolderPath('Desktop')),
        ([Environment]::GetFolderPath('Personal')),
        ($env:USERPROFILE + '\Downloads')
    )
    $zipPath = $null
    foreach ($d in $dirs) {
        if ([string]::IsNullOrEmpty($d)) { continue }
        if (-not (Test-Path -LiteralPath $d)) { continue }
        try {
            Get-ChildItem -LiteralPath $d -Filter 'lobo_edgebug_sound*.zip' -File -ErrorAction Stop | ForEach-Object {
                if (-not $zipPath) {
                    if ($_.Length -eq $needZip) { $zipPath = $_.FullName }
                    else { Write-Log ('[i] ' + $_.FullName + ' is OLD (' + $_.Length + ' bytes, need ' + $needZip + ')') }
                }
            }
        } catch { }
        if ($zipPath) { break }
    }
    if (-not $zipPath) {
        Write-Log '[i] No usable zip found. To repair by hand:'
        Write-Log ('    1. download lobo_edgebug_sound.zip FROM THE CHAT (size must be ' + $needZip + ' bytes)')
        Write-Log '    2. put it next to the source folder (or inside the source folder)'
        Write-Log '    3. run check_source.bat again - it will restore the files by itself'
        return
    }
    Write-Log ('[i] using zip: ' + $zipPath)
    try {
        Add-Type -AssemblyName System.IO.Compression.FileSystem
        $zip = [System.IO.Compression.ZipFile]::OpenRead($zipPath)
        $map = @{ }
        foreach ($e in $zip.Entries) { $map[$e.FullName.ToLowerInvariant()] = $e }
        $todo = @()
        foreach ($x in $script:missList) { $todo += $x }
        foreach ($x in $script:wrongList) { $todo += $x }
        $fixed = 0; $failed = 0
        foreach ($p in $todo) {
            $e = $map[$p.ToLowerInvariant()]
            if (-not $e) {
                Write-Log ('[X] ' + $p + ' is not inside this zip - re-download it from the chat') 'Red'
                $failed++
                continue
            }
            $dest = Join-Path (Get-Location).Path ($p.Replace('/', '\'))
            $dir = Split-Path -Parent $dest
            if ($dir -and -not (Test-Path -LiteralPath $dir)) { New-Item -ItemType Directory -Path $dir -Force | Out-Null }
            try {
                [System.IO.Compression.ZipFileExtensions]::ExtractToFile($e, $dest, $true)
                Write-Log ('[FIXED] restored ' + $p) 'Yellow'
                $fixed++
            } catch {
                Write-Log ('[X] cannot restore ' + $p + ' - close VS / check antivirus (' + $_.Exception.Message + ')') 'Red'
                $failed++
            }
        }
        $zip.Dispose()
        Write-Log ('[i] repaired ' + $fixed + ' file(s), failed ' + $failed)
    } catch {
        Write-Log ('[X] cannot open the zip: ' + $_.Exception.Message) 'Red'
    }
}

Write-Log ''
Write-Log '=== LOBO source checker / fixer v3 ==='
Write-Log ('Folder:   ' + (Get-Location).Path)
Write-Log ('Log file: ' + $logPath)
Write-Log 'All output below is also saved into that log file.'
Write-Log ''

try {

    # ---------- 1) key files ----------
    Write-Log '--- 1. Key files ---'
    $keyItems = @(
        @('csgo-sdk.sln', 'solution'),
        @('build.bat', 'build script'),
        @('check_source.bat', 'this checker launcher'),
        @('hooks\functions\frame_stage_notify.cpp', 'frame hook'),
        @('dependencies\dxsdk-d3dx\include\d3dx9.h', 'D3DX headers'),
        @('Freetype\freetype.lib', 'freetype lib'),
        @('x86-windows-static-md\lib\spdlog.lib', 'vcpkg spdlog lib')
    )
    foreach ($it in $keyItems) {
        if (Test-Path -LiteralPath $it[0]) {
            Write-Log ('[OK] ' + $it[0])
        } else {
            Write-Log ('[X] ' + $it[0] + ' NOT FOUND') 'Red'
            $script:broken = $true
        }
    }

    # ---------- 2) debug.h ----------
    Write-Log ''
    Write-Log '--- 2. hacks\debug\debug.h ---'
    $debugH    = 'hacks\debug\debug.h'
    $debugHLen = 3372
    $needRestore = $false
    if (-not (Test-Path -LiteralPath $debugH)) {
        $needRestore = $true
    } else {
        try {
            if ((Get-Item -LiteralPath $debugH -ErrorAction Stop).Length -ne $debugHLen) { $needRestore = $true }
        } catch { $needRestore = $true }
    }
    if ($needRestore) {
        Write-Log '[!] debug.h is missing or damaged - restoring automatically...' 'Yellow'
        $dir = 'hacks\debug'
        if (-not (Test-Path -LiteralPath $dir)) { New-Item -ItemType Directory -Path $dir -Force | Out-Null }
        $content = @'
#pragma once

#include "../../dependencies/imgui/imgui.h"
#include "../../game/sdk/includes/includes.h"
#include "../../globals/includes/includes.h"
#include "../movement/movement.h"
#include <format>

#ifdef _DEBUG

// debugger menu

namespace n_debugger
{
	struct impl_t {
		void draw_server_hitbox_model( )
		{
			static auto get_player_by_index = []( int index ) {
				static void* pattern = reinterpret_cast< void* >( g_modules[ SERVER_DLL ].find_pattern( "85 C9 7E 32 A1 ? ? ? ?" ) );

				auto util_player_by_index = reinterpret_cast< uintptr_t( __fastcall* )( unsigned int ) >( pattern );

				return util_player_by_index( index );
			};

			static void* pattern =
				reinterpret_cast< void* >( g_modules[ SERVER_DLL ].find_pattern( "55 8B EC 81 EC ? ? ? ? 53 56 8B 35 ? ? ? ? 8B D9 57 8B CE" ) );

			for ( int i = 1; i <= g_interfaces.m_global_vars_base->m_max_clients; i++ ) {
				uintptr_t player = get_player_by_index( i );
				if ( !player )
					return;

				static float duration = -1.0f;

				__asm
				{
					pushad
					movss xmm1, duration
					push 1
					mov ecx, player
					call pattern
					popad
				}
			}
		}

		void on_paint_traverse( )
		{
			if ( !GET_VARIABLE( g_variables.m_debugger_visual, bool ) )
				return;

			float offset = 0.f;

			 auto render_debug = [ & ]( const char* indicator_name, const c_color& color, const bool active ) {
				ImAnimationHelper debug_animation = ImAnimationHelper( fnv1a::hash( indicator_name ), ImGui::GetIO( ).DeltaTime );
				debug_animation.Update( 2.f, active ? 2.f : -2.f );

				if ( debug_animation.AnimationData->second <= 0.f )
					return;

				const auto text_size = g_render.m_fonts[ e_font_names::font_name_verdana_bd_11 ]->CalcTextSizeA(
					g_render.m_fonts[ e_font_names::font_name_verdana_bd_11 ]->FontSize, FLT_MAX, 0.f, indicator_name );

				g_render.m_draw_data.emplace_back(
					e_draw_type::draw_type_text,
					std::make_any< text_draw_object_t >(
						g_render.m_fonts[ e_font_names::font_name_verdana_bd_11 ],
						c_vector_2d( ( g_ctx.m_width / 2 ) + text_size.x, ( g_ctx.m_height / 2 ) - offset - text_size.y ), indicator_name,
						color.get_u32( debug_animation.AnimationData->second ),
						ImColor( 0.f, 0.f, 0.f, color.base< e_color_type::color_type_a >( ) * debug_animation.AnimationData->second ),
						e_text_flags::text_flag_dropshadow ) );

				offset -= ( text_size.y + 2 ) * debug_animation.AnimationData->second;
			};

			std::stringstream ss_eb;
			ss_eb << "eb: duck: " << g_movement.m_edgebug_data.m_edgebug_method << " | delta: " << g_movement.m_edgebug_data.m_yaw_delta;

			auto eb = ss_eb.str( );

			render_debug( eb.c_str( ), c_color( 1.f, 1.f, 1.f, 1.f ), g_movement.m_edgebug_data.m_will_edgebug );

			render_debug( g_movement.m_pixelsurf_data.m_should_duck ? "ducked ps" : "stand ps", c_color( 1.f, 1.f, 1.f, 1.f ),
			              g_movement.m_pixelsurf_data.m_in_pixel_surf || g_movement.m_pixelsurf_data.m_predicted_succesful );
		}

		void on_frame_stage_notify( int stage )
		{
			if ( !GET_VARIABLE( g_variables.m_debugger_visual, bool ) || !g_interfaces.m_engine_client->is_connected_safe( ) || !g_ctx.m_local ||
			     !g_ctx.m_local->is_alive( ) )
				return;

			if ( stage == e_client_frame_stage::net_update_end )
				draw_server_hitbox_model( );
		}
	};
} // namespace n_debugger

inline n_debugger::impl_t g_debugger;
#endif
'@
        try {
            [IO.File]::WriteAllText((Join-Path (Get-Location).Path $debugH), $content + "`n", (New-Object Text.UTF8Encoding($false)))
        } catch {
            Write-Log '[X] cannot write the file - ANTIVIRUS is blocking it!' 'Red'
            Write-Log '    Add this source folder to antivirus exclusions and run again.'
            $script:broken = $true
        }
        if (Test-Path -LiteralPath $debugH) {
            Write-Log '[OK] hacks\debug\debug.h restored!' 'Yellow'
            Write-Log '[i] If it disappears AGAIN - your antivirus (Windows Defender) eats it.'
            Write-Log '    Windows Security -> Virus & threat protection -> Manage settings ->'
            Write-Log '    Exclusions -> Add an exclusion -> Folder -> select THIS source folder.'
            Write-Log '    Then run check_source.bat again.'
        }
    } else {
        Write-Log '[OK] hacks\debug\debug.h is in place'
    }

    # ---------- 3) frame_stage_notify.cpp line 6 ----------
    Write-Log ''
    Write-Log '--- 3. hooks\functions\frame_stage_notify.cpp line 6 ---'
    $f = 'hooks\functions\frame_stage_notify.cpp'
    if (Test-Path -LiteralPath $f) {
        try {
            $canonical = '#include "../../hacks/debug/debug.h"'
            $c = [IO.File]::ReadAllText((Join-Path (Get-Location).Path $f))
            $lines = $c -split "`n"
            $t = $lines[5].Trim()
            Write-Log ('  was: ' + $t)
            $needFix = ($t -ne $canonical) -and ( ($t -match 'debug\.h') -or ($t -notmatch '^#include') )
            if ($needFix) {
                $lines[5] = $canonical
                [IO.File]::WriteAllText((Join-Path (Get-Location).Path $f), ($lines -join "`n"), (New-Object Text.UTF8Encoding($true)))
                Write-Log '  [FIXED] line 6 repaired.' 'Yellow'
            } elseif ($t -eq $canonical) {
                Write-Log '  [OK] include is fine'
            } else {
                Write-Log '  [i] line 6 is a different include, left as is:'
                Write-Log ('      ' + $t)
            }
        } catch {
            Write-Log '  [X] file is too corrupted to read - auto-repair will replace it' 'Red'
            $script:broken = $true
        }
    }

    # ---------- 4) old inventory leftovers (LNK2005 fix) ----------
    Write-Log ''
    Write-Log '--- 4. Old inventory files (caused LNK2005 duplicate errors) ---'
    $obsolete = @(
        'hacks\inventory\inventory_changer.cpp',
        'hacks\inventory\inventory_changer.h',
        'hacks\inventory\items_manager.cpp',
        'hacks\inventory\items_manager.h',
        'hooks\functions\inventory_changer_hooks.cpp'
    )
    $removedOld = 0
    foreach ($o in $obsolete) {
        if (Test-Path -LiteralPath $o) {
            try {
                Remove-Item -LiteralPath $o -Force
                Write-Log ('[FIXED] deleted ' + $o) 'Yellow'
                $removedOld++
            } catch {
                Write-Log ('[X] cannot delete ' + $o + ' - close Visual Studio and run again') 'Red'
                $script:broken = $true
            }
        }
    }
    if (Test-Path -LiteralPath 'hacks\inventory') {
        try {
            $left = @(Get-ChildItem -LiteralPath 'hacks\inventory' -Force -ErrorAction SilentlyContinue)
            if ($left.Count -eq 0) {
                Remove-Item -LiteralPath 'hacks\inventory' -Force
                Write-Log '[FIXED] removed empty folder hacks\inventory' 'Yellow'
            }
        } catch { }
    }
    if ($removedOld -eq 0) { Write-Log '[OK] no old inventory files' }

    # ---------- 5) all source files + auto-repair ----------
    Write-Log ''
    Write-Log '--- 5. Checking every source file (this takes a few seconds) ---'
    $expected = @{ }
    foreach ($line in (@'
.clang-format|1832
Freetype/dfreetype.lib|3729498
Freetype/freetype.lib|4122502
Freetype/freetype/config/ftconfig.h|1614
Freetype/freetype/config/ftheader.h|23919
Freetype/freetype/config/ftmodule.h|1443
Freetype/freetype/config/ftoption.h|40350
Freetype/freetype/config/ftstdlib.h|4576
Freetype/freetype/config/integer-types.h|7072
Freetype/freetype/config/mac-support.h|1597
Freetype/freetype/config/public-macros.h|4207
Freetype/freetype/freetype.h|177200
Freetype/freetype/ftadvanc.h|5470
Freetype/freetype/ftbbox.h|2638
Freetype/freetype/ftbdf.h|5322
Freetype/freetype/ftbitmap.h|9051
Freetype/freetype/ftbzip2.h|2786
Freetype/freetype/ftcache.h|34179
Freetype/freetype/ftchapters.h|2933
Freetype/freetype/ftcid.h|4022
Freetype/freetype/ftcolor.h|50199
Freetype/freetype/ftdriver.h|50575
Freetype/freetype/fterrdef.h|12559
Freetype/freetype/fterrors.h|9301
Freetype/freetype/ftfntfmt.h|2213
Freetype/freetype/ftgasp.h|4138
Freetype/freetype/ftglyph.h|20912
Freetype/freetype/ftgxval.h|10625
Freetype/freetype/ftgzip.h|4211
Freetype/freetype/ftimage.h|42136
Freetype/freetype/ftincrem.h|10696
Freetype/freetype/ftlcdfil.h|11744
Freetype/freetype/ftlist.h|7100
Freetype/freetype/ftlogging.h|4130
Freetype/freetype/ftlzw.h|2768
Freetype/freetype/ftmac.h|7771
Freetype/freetype/ftmm.h|24595
Freetype/freetype/ftmodapi.h|22544
Freetype/freetype/ftmoderr.h|6675
Freetype/freetype/ftotval.h|5346
Freetype/freetype/ftoutln.h|17403
Freetype/freetype/ftparams.h|6041
Freetype/freetype/ftpfr.h|4908
Freetype/freetype/ftrender.h|6625
Freetype/freetype/ftsizes.h|4288
Freetype/freetype/ftsnames.h|7730
Freetype/freetype/ftstroke.h|21773
Freetype/freetype/ftsynth.h|4483
Freetype/freetype/ftsystem.h|8502
Freetype/freetype/fttrigon.h|7411
Freetype/freetype/fttypes.h|14735
Freetype/freetype/ftwinfnt.h|7965
Freetype/freetype/internal/autohint.h|7498
Freetype/freetype/internal/cffotypes.h|2354
Freetype/freetype/internal/cfftypes.h|11866
Freetype/freetype/internal/compiler-macros.h|11499
Freetype/freetype/internal/ftcalc.h|15749
Freetype/freetype/internal/ftdebug.h|13332
Freetype/freetype/internal/ftdrv.h|9214
Freetype/freetype/internal/ftgloadr.h|4504
Freetype/freetype/internal/fthash.h|3242
Freetype/freetype/internal/ftmemory.h|16123
Freetype/freetype/internal/ftmmtypes.h|3003
Freetype/freetype/internal/ftobjs.h|42481
Freetype/freetype/internal/ftpsprop.h|1130
Freetype/freetype/internal/ftrfork.h|7651
Freetype/freetype/internal/ftserv.h|24053
Freetype/freetype/internal/ftstream.h|23230
Freetype/freetype/internal/fttrace.h|5987
Freetype/freetype/internal/ftvalid.h|5762
Freetype/freetype/internal/psaux.h|43936
Freetype/freetype/internal/pshints.h|20852
Freetype/freetype/internal/services/svbdf.h|1731
Freetype/freetype/internal/services/svcfftl.h|2832
Freetype/freetype/internal/services/svcid.h|2124
Freetype/freetype/internal/services/svfntfmt.h|1348
Freetype/freetype/internal/services/svgldict.h|1865
Freetype/freetype/internal/services/svgxval.h|1730
Freetype/freetype/internal/services/svkern.h|1080
Freetype/freetype/internal/services/svmetric.h|3996
Freetype/freetype/internal/services/svmm.h|8962
Freetype/freetype/internal/services/svotval.h|1253
Freetype/freetype/internal/services/svpfr.h|1570
Freetype/freetype/internal/services/svpostnm.h|1571
Freetype/freetype/internal/services/svprop.h|1677
Freetype/freetype/internal/services/svpscmap.h|4272
Freetype/freetype/internal/services/svpsinfo.h|2469
Freetype/freetype/internal/services/svsfnt.h|2079
Freetype/freetype/internal/services/svttcmap.h|2397
Freetype/freetype/internal/services/svtteng.h|1007
Freetype/freetype/internal/services/svttglyf.h|1251
Freetype/freetype/internal/services/svwinfnt.h|1017
Freetype/freetype/internal/sfnt.h|32698
Freetype/freetype/internal/svginterface.h|1041
Freetype/freetype/internal/t1types.h|9563
Freetype/freetype/internal/tttypes.h|52183
Freetype/freetype/internal/wofftypes.h|8010
Freetype/freetype/otsvg.h|10457
Freetype/freetype/t1tables.h|21529
Freetype/freetype/ttnameid.h|58769
Freetype/freetype/tttables.h|25227
Freetype/freetype/tttags.h|5145
Freetype/ft2build.h|990
build.bat|3486
check_source.bat|476
csgo-sdk.sln|1405
csgo-sdk.vcxproj|43888
csgo-sdk.vcxproj.filters|32923
csgo-sdk.vcxproj.user|225
delete_cache.bat|181
dependencies/avatar_data/avatar_data.h|514493
dependencies/crc32/crc32.cpp|5879
dependencies/crc32/crc32.h|433
dependencies/dxsdk-d3dx/LICENSE.txt|11846
dependencies/dxsdk-d3dx/bin/x86/D3DX9_43.dll|2001304
dependencies/dxsdk-d3dx/include/D3DX10.h|2103
dependencies/dxsdk-d3dx/include/D3DX10core.h|14454
dependencies/dxsdk-d3dx/include/D3DX10math.h|62479
dependencies/dxsdk-d3dx/include/D3DX10math.inl|45816
dependencies/dxsdk-d3dx/include/D3DX10mesh.h|11046
dependencies/dxsdk-d3dx/include/D3DX10tex.h|26616
dependencies/dxsdk-d3dx/include/D3DX11.h|2100
dependencies/dxsdk-d3dx/include/D3DX11async.h|8700
dependencies/dxsdk-d3dx/include/D3DX11core.h|3868
dependencies/dxsdk-d3dx/include/D3DX11tex.h|26986
dependencies/dxsdk-d3dx/include/D3DX_DXGIFormatConvert.inl|34332
dependencies/dxsdk-d3dx/include/d3dx10async.h|15605
dependencies/dxsdk-d3dx/include/d3dx9.h|2385
dependencies/dxsdk-d3dx/include/d3dx9anim.h|43369
dependencies/dxsdk-d3dx/include/d3dx9core.h|23921
dependencies/dxsdk-d3dx/include/d3dx9effect.h|42312
dependencies/dxsdk-d3dx/include/d3dx9math.h|59604
dependencies/dxsdk-d3dx/include/d3dx9math.inl|45267
dependencies/dxsdk-d3dx/include/d3dx9mesh.h|128409
dependencies/dxsdk-d3dx/include/d3dx9shader.h|39300
dependencies/dxsdk-d3dx/include/d3dx9shape.h|7943
dependencies/dxsdk-d3dx/include/d3dx9tex.h|61840
dependencies/dxsdk-d3dx/include/d3dx9xof.h|12012
dependencies/dxsdk-d3dx/include/rmxfguid.h|8751
dependencies/dxsdk-d3dx/include/rmxftmpl.h|17623
dependencies/dxsdk-d3dx/lib/x64/d3dx9.lib|81324
dependencies/dxsdk-d3dx/lib/x86/d3dx9.lib|87600
dependencies/fnv1a/fnv1a.h|611
dependencies/fonts/anime.h|47788
dependencies/fonts/apple-font/SF-Pro-Display-Black.h|1937580
dependencies/fonts/apple-font/SF-Pro-Display-Bold.h|2029390
dependencies/fonts/apple-font/SF-Pro-Display-Heavy.h|2036132
dependencies/fonts/apple-font/SF-Pro-Display-Light.h|1933482
dependencies/fonts/apple-font/SF-Pro-Display-Medium.h|2034145
dependencies/fonts/apple-font/SF-Pro-Display-Regular.h|1812452
dependencies/fonts/apple-font/SF-Pro-Display-Semibold.h|2036887
dependencies/fonts/apple-font/SF-Pro-Display-Thin.h|1926376
dependencies/fonts/apple-font/SF-Pro-Display-Ultralight.h|1919640
dependencies/fonts/lobotomy.h|35212
dependencies/imgui/helpers/easing.h|3815
dependencies/imgui/helpers/fonts.h|5528860
dependencies/imgui/imconfig.h|9664
dependencies/imgui/imgui.cpp|703274
dependencies/imgui/imgui.h|298470
dependencies/imgui/imgui_demo.cpp|413532
dependencies/imgui/imgui_draw.cpp|211003
dependencies/imgui/imgui_freetype.cpp|35522
dependencies/imgui/imgui_freetype.h|3542
dependencies/imgui/imgui_impl_dx9.cpp|14023
dependencies/imgui/imgui_impl_dx9.h|1035
dependencies/imgui/imgui_impl_win32.cpp|15855
dependencies/imgui/imgui_impl_win32.h|1391
dependencies/imgui/imgui_internal.h|216766
dependencies/imgui/imgui_tables.cpp|217490
dependencies/imgui/imgui_widgets.cpp|416738
dependencies/imgui/imstb_rectpack.h|20344
dependencies/imgui/imstb_textedit.h|54933
dependencies/imgui/imstb_truetype.h|199384
dependencies/json/adl_serializer.hpp|1486
dependencies/json/detail/conversions/from_json.hpp|14072
dependencies/json/detail/conversions/to_chars.hpp|37685
dependencies/json/detail/conversions/to_json.hpp|11632
dependencies/json/detail/exceptions.hpp|21203
dependencies/json/detail/input/binary_reader.hpp|61001
dependencies/json/detail/input/input_adapters.hpp|16385
dependencies/json/detail/input/json_sax.hpp|20636
dependencies/json/detail/input/lexer.hpp|49864
dependencies/json/detail/input/parser.hpp|18461
dependencies/json/detail/input/position_t.hpp|605
dependencies/json/detail/iterators/internal_iterator.hpp|693
dependencies/json/detail/iterators/iter_impl.hpp|18558
dependencies/json/detail/iterators/iteration_proxy.hpp|5432
dependencies/json/detail/iterators/iterator_traits.hpp|1382
dependencies/json/detail/iterators/json_reverse_iterator.hpp|3467
dependencies/json/detail/iterators/primitive_iterator.hpp|2802
dependencies/json/detail/json_pointer.hpp|35017
dependencies/json/detail/json_ref.hpp|1657
dependencies/json/detail/macro_scope.hpp|6036
dependencies/json/detail/macro_unscope.hpp|481
dependencies/json/detail/meta/cpp_future.hpp|1862
dependencies/json/detail/meta/detected.hpp|1686
dependencies/json/detail/meta/is_sax.hpp|6133
dependencies/json/detail/meta/type_traits.hpp|14164
dependencies/json/detail/meta/void_t.hpp|246
dependencies/json/detail/output/binary_writer.hpp|49107
dependencies/json/detail/output/output_adapters.hpp|3090
dependencies/json/detail/output/serializer.hpp|34094
dependencies/json/detail/value_t.hpp|2818
dependencies/json/json.hpp|296054
dependencies/json/json_fwd.hpp|1833
dependencies/json/thirdparty/hedley/hedley.hpp|64097
dependencies/json/thirdparty/hedley/hedley_undef.hpp|4416
dependencies/lazy_importer/lazy_importer.h|22132
dependencies/minhook/include/MinHook.h|7530
dependencies/minhook/src/buffer.c|9814
dependencies/minhook/src/buffer.h|1820
dependencies/minhook/src/hde/hde32.c|9318
dependencies/minhook/src/hde/hde32.h|2450
dependencies/minhook/src/hde/hde64.c|9507
dependencies/minhook/src/hde/hde64.h|2602
dependencies/minhook/src/hde/pstdint.h|1662
dependencies/minhook/src/hde/table32.h|3508
dependencies/minhook/src/hde/table64.h|3577
dependencies/minhook/src/hook.c|26128
dependencies/minhook/src/trampoline.c|10631
dependencies/minhook/src/trampoline.h|3759
dependencies/steam/isteamapplist.h|2475
dependencies/steam/isteamapps.h|7626
dependencies/steam/isteamappticket.h|1233
dependencies/steam/isteamclient.h|21323
dependencies/steam/isteamcontroller.h|20562
dependencies/steam/isteamfriends.h|29806
dependencies/steam/isteamgamecoordinator.h|2690
dependencies/steam/isteamgameserver.h|16211
dependencies/steam/isteamgameserverstats.h|4388
dependencies/steam/isteamhtmlsurface.h|22861
dependencies/steam/isteamhttp.h|11396
dependencies/steam/isteaminventory.h|23636
dependencies/steam/isteammasterserverupdater.h|39
dependencies/steam/isteammatchmaking.h|39356
dependencies/steam/isteammusic.h|1682
dependencies/steam/isteammusicremote.h|4295
dependencies/steam/isteamnetworking.h|16215
dependencies/steam/isteamparentalsettings.h|1537
dependencies/steam/isteamps3overlayrenderer.h|3714
dependencies/steam/isteamremotestorage.h|32028
dependencies/steam/isteamscreenshots.h|5146
dependencies/steam/isteamugc.h|28116
dependencies/steam/isteamuser.h|17751
dependencies/steam/isteamuserstats.h|24132
dependencies/steam/isteamutils.h|11991
dependencies/steam/isteamvideo.h|1983
dependencies/steam/matchmakingtypes.h|7642
dependencies/steam/steam_api.h|17296
dependencies/steam/steam_api.json|285850
dependencies/steam/steam_api_flat.h|95194
dependencies/steam/steam_api_internal.h|13448
dependencies/steam/steam_api_interop.cs|558104
dependencies/steam/steam_gameserver.h|11798
dependencies/steam/steamclientpublic.h|56063
dependencies/steam/steamencryptedappticket.h|1669
dependencies/steam/steamhttpenums.h|3590
dependencies/steam/steamps3params.h|3615
dependencies/steam/steamtypes.h|5745
dependencies/steam/steamuniverse.h|572
edgebug_sound/README_EDGEBUG_SOUND_RU.txt|20129
edgebug_sound/edgebug.wav|24740
elements/button.hh|4374
elements/checkbox.hh|13234
elements/childbox.hh|3214
elements/combobox.hh|9324
elements/i_text.hh|291
elements/sliders.hh|7117
elements/spinner.hh|1988
elements/tabs.hh|6975
game/datatypes/fn_change_callback.h|90
game/datatypes/recv_var_proxy_fn.h|125
game/sdk/classes/c_angle.cpp|1206
game/sdk/classes/c_angle.h|4395
game/sdk/classes/c_animation_state.h|1447
game/sdk/classes/c_app_system.h|950
game/sdk/classes/c_base_client.h|444
game/sdk/classes/c_base_client_dll.h|214
game/sdk/classes/c_breakable_with_prop_data.h|2060
game/sdk/classes/c_cconvar.h|1524
game/sdk/classes/c_client_entity_list.h|1356
game/sdk/classes/c_client_state.h|2380
game/sdk/classes/c_color.cpp|535
game/sdk/classes/c_color.h|4205
game/sdk/classes/c_con_base.h|205
game/sdk/classes/c_convar.h|2518
game/sdk/classes/c_csgo_hud_radar.h|528
game/sdk/classes/c_debug_overlay.h|5388
game/sdk/classes/c_engine_client.h|4110
game/sdk/classes/c_engine_sound.h|645
game/sdk/classes/c_engine_trace.h|14717
game/sdk/classes/c_game_movement.h|1122
game/sdk/classes/c_game_types.h|171
game/sdk/classes/c_global_vars_base.h|473
game/sdk/classes/c_input.h|798
game/sdk/classes/c_input_system.h|180
game/sdk/classes/c_key_values.cpp|5999
game/sdk/classes/c_key_values.h|2351
game/sdk/classes/c_key_values_system.h|1286
game/sdk/classes/c_localize.h|318
game/sdk/classes/c_material.h|6807
game/sdk/classes/c_material_system.h|10537
game/sdk/classes/c_material_var.h|846
game/sdk/classes/c_model_cache.h|1652
game/sdk/classes/c_model_info.h|7513
game/sdk/classes/c_model_render.h|514
game/sdk/classes/c_move_data.h|883
game/sdk/classes/c_move_helper.h|2076
game/sdk/classes/c_multiplayer_physics.h|200
game/sdk/classes/c_net_channel_info.h|440
game/sdk/classes/c_net_message.h|802
game/sdk/classes/c_network_string_table.h|2496
game/sdk/classes/c_panel.h|193
game/sdk/classes/c_physics_collisons.h|5641
game/sdk/classes/c_physics_surface_props.h|3071
game/sdk/classes/c_prediction.h|1757
game/sdk/classes/c_recv_proxy_data.h|226
game/sdk/classes/c_standart_recv_proxies.h|326
game/sdk/classes/c_studio_render.h|289
game/sdk/classes/c_surface.h|336
game/sdk/classes/c_user_cmd.cpp|1500
game/sdk/classes/c_user_cmd.h|839
game/sdk/classes/c_utl_memory.h|1939
game/sdk/classes/c_utl_reference.h|128
game/sdk/classes/c_utl_string.h|105
game/sdk/classes/c_utl_vector.h|1339
game/sdk/classes/c_vector.h|9032
game/sdk/classes/c_view_setup.h|1363
game/sdk/classes/c_weapon_data.cpp|422
game/sdk/classes/c_weapon_data.h|2664
game/sdk/classes/c_weapon_system.h|263
game/sdk/classes/econ_item.cpp|3225
game/sdk/classes/econ_item.h|2466
game/sdk/classes/entity.cpp|16909
game/sdk/classes/entity.h|27378
game/sdk/classes/i_player_inventory.cpp|2944
game/sdk/classes/i_player_inventory.h|9109
game/sdk/classes/i_shared_object.h|0
game/sdk/classes/panorama/c_panorama_ui_engine.h|19441
game/sdk/classes/panorama/c_uievent.h|1320
game/sdk/classes/panorama/c_uipanel.h|32646
game/sdk/classes/panorama/panorama.h|1080
game/sdk/enums/e_class_ids.h|5411
game/sdk/enums/e_client_frame_stage.h|239
game/sdk/enums/e_collision_group.h|817
game/sdk/enums/e_command_buttons.h|955
game/sdk/enums/e_convar_flag.h|3399
game/sdk/enums/e_damage_type.h|105
game/sdk/enums/e_field_types.h|599
game/sdk/enums/e_flags.h|1338
game/sdk/enums/e_glow_style.h|167
game/sdk/enums/e_hitgroup.h|654
game/sdk/enums/e_image_format.h|2092
game/sdk/enums/e_item_definition_index.h|5574
game/sdk/enums/e_material_property_type.h|196
game/sdk/enums/e_material_var_flags.h|1904
game/sdk/enums/e_move_types.h|508
game/sdk/enums/e_multiplayer_break.h|163
game/sdk/enums/e_multiplayer_physics_mode.h|204
game/sdk/enums/e_obs_mode.h|172
game/sdk/enums/e_preview_image_ret_val.h|142
game/sdk/enums/e_property_data_interactions.h|742
game/sdk/enums/e_send_prop_type.h|175
game/sdk/enums/e_team_id.h|97
game/sdk/enums/e_valve_decals.h|710
game/sdk/enums/e_weapon_type.h|545
game/sdk/includes/includes.h|3356
game/sdk/structs/bounding_box_t.h|134
game/sdk/structs/data_map_t.h|254
game/sdk/structs/data_variant_t.h|214
game/sdk/structs/fmt_str_n_t.h|283
game/sdk/structs/game_event_t.h|3057
game/sdk/structs/interface_node_t.h|125
game/sdk/structs/matrix_t.h|9069
game/sdk/structs/model_render_info_t.h|464
game/sdk/structs/mstudiobone_t.h|640
game/sdk/structs/mstudiohitboxset_t.h|852
game/sdk/structs/player_info_t.h|427
game/sdk/structs/rect_t.h|84
game/sdk/structs/recv_prop_t.h|704
game/sdk/structs/recv_table_t.h|189
game/sdk/structs/renderable_info_t.h|516
game/sdk/structs/string_t.h|581
game/sdk/structs/studiohdr_t.h|6898
game/sdk/structs/type_description_t.h|292
game/sdk/structs/var_mapping.h|255
game/sdk/structs/vcollide_t.h|195
globals/config/config.cpp|10157
globals/config/config.h|2976
globals/config/structs/font_setting_t.h|98
globals/config/variables.h|28712
globals/convars/convars.cpp|850
globals/convars/convars.h|207
globals/fonts/fonts.cpp|765
globals/fonts/fonts.h|222
globals/globals.cpp|744
globals/globals.h|964
globals/includes/includes.h|1539
globals/interfaces/interfaces.cpp|7522
globals/interfaces/interfaces.h|3308
globals/logger/debug_log.cpp|1712
globals/logger/logger.h|3338
globals/macros/macros.h|3525
globals/math/math.cpp|4453
globals/math/math.h|1511
globals/media_player/media_player.cpp|3500
globals/media_player/media_player.h|1459
globals/netvars/netvars.cpp|2899
globals/netvars/netvars.h|6474
globals/render/draw.cpp|3883
globals/render/draw.h|1323
globals/render/render.cpp|21640
globals/render/render.h|6178
hacks/aimbot/aimbot.cpp|14343
hacks/aimbot/aimbot.h|1023
hacks/animations/animations.cpp|830
hacks/animations/animations.h|177
hacks/auto_wall/auto_wall.cpp|13895
hacks/auto_wall/auto_wall.h|1490
hacks/avatar_cache/avatar_cache.cpp|1511
hacks/avatar_cache/avatar_cache.h|445
hacks/debug/debug.h|3372
hacks/entity_cache/entity_cache.cpp|1719
hacks/entity_cache/entity_cache.h|476
hacks/indicators/indicators.cpp|37516
hacks/indicators/indicators.h|1300
hacks/lagcomp/lagcomp.cpp|7144
hacks/lagcomp/lagcomp.h|1889
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-13511-1.gif|25407
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-13511-13.gif|24265
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-13511-2.gif|21570
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-13511-24.gif|25530
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-13511-35.gif|28397
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-13511-46.gif|27169
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-13511-57.gif|27127
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-13511-68.gif|26940
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-13511-71.gif|26915
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-13511-72.gif|27013
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-10.gif|24331
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-11.gif|24180
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-12.gif|23913
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-14.gif|24010
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-15.gif|22578
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-16.gif|21798
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-17.gif|21541
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-18.gif|24790
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-19.gif|24790
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-20.gif|25267
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-21.gif|25146
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-22.gif|25117
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-23.gif|25160
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-25.gif|24816
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-26.gif|24867
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-27.gif|19765
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-28.gif|19864
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-29.gif|23378
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-3.gif|26443
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-30.gif|22865
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-31.gif|23922
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-32.gif|23858
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-33.gif|26201
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-34.gif|26211
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-36.gif|25932
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-37.gif|25939
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-38.gif|24422
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-39.gif|22167
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-4.gif|26537
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-40.gif|22088
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-41.gif|22128
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-42.gif|24608
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-43.gif|24583
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-44.gif|23235
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-45.gif|23340
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-47.gif|23340
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-48.gif|21757
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-49.gif|21814
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-5.gif|25599
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-50.gif|21696
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-51.gif|23515
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-52.gif|23576
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-53.gif|27578
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-54.gif|27522
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-55.gif|27568
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-56.gif|27473
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-58.gif|27453
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-59.gif|26902
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-6.gif|23162
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-60.gif|26806
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-61.gif|24910
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-62.gif|24958
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-63.gif|24827
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-64.gif|24745
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-65.gif|24811
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-66.gif|25310
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-67.gif|25255
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-69.gif|25250
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-7.gif|21732
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-70.gif|25184
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-8.gif|21782
hacks/menu/anime/99dbe8646733d4c63c113fe6fe8a44d3-imageonline.co-135115-9.gif|23763
hacks/menu/menu.cpp|133880
hacks/menu/menu.h|694
hacks/misc/misc.cpp|38072
hacks/misc/misc.h|904
hacks/misc/scaleform/files/alerts.js|6906
hacks/misc/scaleform/files/deathnotices.js|3992
hacks/misc/scaleform/files/health_and_ammo.js|5944
hacks/misc/scaleform/files/teamcount_avatar.js|5443
hacks/misc/scaleform/files/teamcount_score.js|3225
hacks/misc/scaleform/files/weapon_select.js|2152
hacks/misc/scaleform/scaleform.cpp|25613
hacks/misc/scaleform/scaleform.h|1718
hacks/movement/air_rollout.h|8118
hacks/movement/auto_bounce.cpp|55731
hacks/movement/edgebug.cpp|28850
hacks/movement/edgebug.h|598
hacks/movement/movement.cpp|317704
hacks/movement/movement.h|6024
hacks/movement/texturebug.cpp|75178
hacks/movement/texturebug.h|1144
hacks/movement/texturebug_common.h|10890
hacks/movement/texturebug_wallstrafe.h|4416
hacks/movement/tick_scale.h|3500
hacks/prediction/prediction.cpp|6217
hacks/prediction/prediction.h|1075
hacks/skins/parser.cpp|1000
hacks/skins/skin_names.h|38206
hacks/skins/skins.cpp|16889
hacks/skins/skins.h|7971
hacks/skins/skins_changers.cpp|29141
hacks/skins/skins_internal.h|63992
hacks/skins/sticker_names.h|298551
hacks/visuals/edicts/edicts.cpp|19279
hacks/visuals/edicts/edicts.h|8261
hacks/visuals/players/chams/chams.h|13
hacks/visuals/players/dormancy/dormancy.cpp|2406
hacks/visuals/players/dormancy/dormancy.h|1523
hacks/visuals/players/esp.cpp|0
hacks/visuals/players/esp.h|0
hacks/visuals/players/players.cpp|22699
hacks/visuals/players/players.h|679
hacks/visuals/screen/screen.cpp|11521
hacks/visuals/screen/screen.h|358
hooks/detour_hook/c_detour_hook.cpp|1184
hooks/detour_hook/c_detour_hook.h|544
hooks/detour_hook/vfunc.h|1065
hooks/functions/alloc_key_values_memory.cpp|1054
hooks/functions/create_move_proxy.cpp|4290
hooks/functions/draw_model_execute.cpp|11902
hooks/functions/draw_set_color.cpp|534
hooks/functions/draw_view_models.cpp|476
hooks/functions/emit_sound.cpp|1055
hooks/functions/end_scene.cpp|1138
hooks/functions/find_material.cpp|806
hooks/functions/find_mdl.cpp|529
hooks/functions/fire_event_intern.cpp|4758
hooks/functions/frame_stage_notify.cpp|2312
hooks/functions/get_vcollide.cpp|401
hooks/functions/glow_effect_spectator.cpp|1302
hooks/functions/is_hltv.cpp|883
hooks/functions/is_paused.cpp|700
hooks/functions/is_playing_demo.cpp|749
hooks/functions/level_init_post_entity.cpp|469
hooks/functions/level_init_pre_entity.cpp|881
hooks/functions/level_shutdown.cpp|535
hooks/functions/list_leaves_in_box.cpp|1700
hooks/functions/lock_cursor.cpp|450
hooks/functions/modify_eye_position.cpp|770
hooks/functions/net_earliertempents.cpp|624
hooks/functions/on_add_entity.cpp|742
hooks/functions/on_remove_entity.cpp|757
hooks/functions/override_mouse_input.cpp|1017
hooks/functions/paint_traverse.cpp|2444
hooks/functions/particle_collection_simulate.cpp|4389
hooks/functions/process_movement.cpp|566
hooks/functions/reset.cpp|598
hooks/functions/run_command.cpp|480
hooks/functions/send_datagram.cpp|1524
hooks/functions/send_net_msg.cpp|457
hooks/functions/set_image_data_r8g8b8a8.cpp|2342
hooks/functions/set_visuals_data.cpp|1587
hooks/functions/wndproc.cpp|683
hooks/hooks.cpp|7703
hooks/hooks.h|6103
hooks/recv.h|3670
instalisation fonts/fonts2.cpp|6017
instalisation fonts/fonts2.h|565
main.cpp|13345
tooltips/tooltips.h|1222
utilities/console/console.cpp|490
utilities/console/console.h|617
utilities/console/console.inl|574
utilities/data/data.h|1540227
utilities/input/input.cpp|3121
utilities/input/input.h|902
utilities/input/key_bind_t.h|73
utilities/memory/memory.cpp|19
utilities/memory/memory.h|0
utilities/memory/relative.h|246
utilities/memory/structs/pe32.h|17887
utilities/memory/virtual.h|537
utilities/modules/modules.cpp|5322
utilities/modules/modules.h|738
utilities/utilities.cpp|18415
utilities/utilities.h|4021
x86-windows-static-md/debug/lib/fmtd.lib|2668212
x86-windows-static-md/debug/lib/pkgconfig/fmt.pc|241
x86-windows-static-md/debug/lib/pkgconfig/spdlog.pc|354
x86-windows-static-md/debug/lib/spdlogd.lib|14055992
x86-windows-static-md/include/fmt/args.h|7437
x86-windows-static-md/include/fmt/base.h|104877
x86-windows-static-md/include/fmt/chrono.h|80448
x86-windows-static-md/include/fmt/color.h|23430
x86-windows-static-md/include/fmt/compile.h|18651
x86-windows-static-md/include/fmt/core.h|187
x86-windows-static-md/include/fmt/format-inl.h|80577
x86-windows-static-md/include/fmt/format.h|163131
x86-windows-static-md/include/fmt/os.h|13041
x86-windows-static-md/include/fmt/ostream.h|6156
x86-windows-static-md/include/fmt/printf.h|20183
x86-windows-static-md/include/fmt/ranges.h|29395
x86-windows-static-md/include/fmt/std.h|21124
x86-windows-static-md/include/fmt/xchar.h|12035
x86-windows-static-md/include/spdlog/async.h|4021
x86-windows-static-md/include/spdlog/async_logger-inl.h|2817
x86-windows-static-md/include/spdlog/async_logger.h|2560
x86-windows-static-md/include/spdlog/cfg/argv.h|1219
x86-windows-static-md/include/spdlog/cfg/env.h|1007
x86-windows-static-md/include/spdlog/cfg/helpers-inl.h|3170
x86-windows-static-md/include/spdlog/cfg/helpers.h|718
x86-windows-static-md/include/spdlog/common-inl.h|2050
x86-windows-static-md/include/spdlog/common.h|13054
x86-windows-static-md/include/spdlog/details/backtracer-inl.h|2004
x86-windows-static-md/include/spdlog/details/backtracer.h|1165
x86-windows-static-md/include/spdlog/details/circular_q.h|3377
x86-windows-static-md/include/spdlog/details/console_globals.h|603
x86-windows-static-md/include/spdlog/details/file_helper-inl.h|4709
x86-windows-static-md/include/spdlog/details/file_helper.h|1758
x86-windows-static-md/include/spdlog/details/fmt_helper.h|4471
x86-windows-static-md/include/spdlog/details/log_msg-inl.h|1396
x86-windows-static-md/include/spdlog/details/log_msg.h|1204
x86-windows-static-md/include/spdlog/details/log_msg_buffer-inl.h|1668
x86-windows-static-md/include/spdlog/details/log_msg_buffer.h|935
x86-windows-static-md/include/spdlog/details/mpmc_blocking_q.h|5282
x86-windows-static-md/include/spdlog/details/null_mutex.h|930
x86-windows-static-md/include/spdlog/details/os-inl.h|18676
x86-windows-static-md/include/spdlog/details/os.h|4110
x86-windows-static-md/include/spdlog/details/periodic_worker-inl.h|620
x86-windows-static-md/include/spdlog/details/periodic_worker.h|1813
x86-windows-static-md/include/spdlog/details/registry-inl.h|8551
x86-windows-static-md/include/spdlog/details/registry.h|4331
x86-windows-static-md/include/spdlog/details/synchronous_factory.h|751
x86-windows-static-md/include/spdlog/details/tcp_client-windows.h|4252
x86-windows-static-md/include/spdlog/details/tcp_client.h|3885
x86-windows-static-md/include/spdlog/details/thread_pool-inl.h|4275
x86-windows-static-md/include/spdlog/details/thread_pool.h|3519
x86-windows-static-md/include/spdlog/details/udp_client-windows.h|3134
x86-windows-static-md/include/spdlog/details/udp_client.h|2237
x86-windows-static-md/include/spdlog/details/windows_include.h|188
x86-windows-static-md/include/spdlog/fmt/bin_to_hex.h|7374
x86-windows-static-md/include/spdlog/fmt/chrono.h|551
x86-windows-static-md/include/spdlog/fmt/compile.h|559
x86-windows-static-md/include/spdlog/fmt/fmt.h|825
x86-windows-static-md/include/spdlog/fmt/ostr.h|554
x86-windows-static-md/include/spdlog/fmt/ranges.h|551
x86-windows-static-md/include/spdlog/fmt/std.h|641
x86-windows-static-md/include/spdlog/fmt/xchar.h|548
x86-windows-static-md/include/spdlog/formatter.h|463
x86-windows-static-md/include/spdlog/fwd.h|305
x86-windows-static-md/include/spdlog/logger-inl.h|6725
x86-windows-static-md/include/spdlog/logger.h|13412
x86-windows-static-md/include/spdlog/mdc.h|1491
x86-windows-static-md/include/spdlog/pattern_formatter-inl.h|46644
x86-windows-static-md/include/spdlog/pattern_formatter.h|3758
x86-windows-static-md/include/spdlog/sinks/android_sink.h|4777
x86-windows-static-md/include/spdlog/sinks/ansicolor_sink-inl.h|5094
x86-windows-static-md/include/spdlog/sinks/ansicolor_sink.h|4029
x86-windows-static-md/include/spdlog/sinks/base_sink-inl.h|1808
x86-windows-static-md/include/spdlog/sinks/base_sink.h|1604
x86-windows-static-md/include/spdlog/sinks/basic_file_sink-inl.h|1391
x86-windows-static-md/include/spdlog/sinks/basic_file_sink.h|2316
x86-windows-static-md/include/spdlog/sinks/callback_sink.h|1704
x86-windows-static-md/include/spdlog/sinks/daily_file_sink.h|9610
x86-windows-static-md/include/spdlog/sinks/dist_sink.h|2373
x86-windows-static-md/include/spdlog/sinks/dup_filter_sink.h|3243
x86-windows-static-md/include/spdlog/sinks/hourly_file_sink.h|7279
x86-windows-static-md/include/spdlog/sinks/kafka_sink.h|4445
x86-windows-static-md/include/spdlog/sinks/mongo_sink.h|4003
x86-windows-static-md/include/spdlog/sinks/msvc_sink.h|2048
x86-windows-static-md/include/spdlog/sinks/null_sink.h|1240
x86-windows-static-md/include/spdlog/sinks/ostream_sink.h|1222
x86-windows-static-md/include/spdlog/sinks/qt_sinks.h|12412
x86-windows-static-md/include/spdlog/sinks/ringbuffer_sink.h|2081
x86-windows-static-md/include/spdlog/sinks/rotating_file_sink-inl.h|5100
x86-windows-static-md/include/spdlog/sinks/rotating_file_sink.h|3252
x86-windows-static-md/include/spdlog/sinks/sink-inl.h|725
x86-windows-static-md/include/spdlog/sinks/sink.h|893
x86-windows-static-md/include/spdlog/sinks/stdout_color_sinks-inl.h|1435
x86-windows-static-md/include/spdlog/sinks/stdout_color_sinks.h|1792
x86-windows-static-md/include/spdlog/sinks/stdout_sinks-inl.h|4384
x86-windows-static-md/include/spdlog/sinks/stdout_sinks.h|2429
x86-windows-static-md/include/spdlog/sinks/syslog_sink.h|4248
x86-windows-static-md/include/spdlog/sinks/systemd_sink.h|4872
x86-windows-static-md/include/spdlog/sinks/tcp_sink.h|2127
x86-windows-static-md/include/spdlog/sinks/udp_sink.h|1858
x86-windows-static-md/include/spdlog/sinks/win_eventlog_sink.h|8888
x86-windows-static-md/include/spdlog/sinks/wincolor_sink-inl.h|7044
x86-windows-static-md/include/spdlog/sinks/wincolor_sink.h|2716
x86-windows-static-md/include/spdlog/spdlog-inl.h|3118
x86-windows-static-md/include/spdlog/spdlog.h|11771
x86-windows-static-md/include/spdlog/stopwatch.h|1865
x86-windows-static-md/include/spdlog/tweakme.h|6792
x86-windows-static-md/include/spdlog/version.h|417
x86-windows-static-md/lib/fmt.lib|3237422
x86-windows-static-md/lib/pkgconfig/fmt.pc|237
x86-windows-static-md/lib/pkgconfig/spdlog.pc|351
x86-windows-static-md/lib/spdlog.lib|14724918
x86-windows-static-md/share/fmt/copyright|1431
x86-windows-static-md/share/fmt/fmt-config-version.cmake|1905
x86-windows-static-md/share/fmt/fmt-config.cmake|1027
x86-windows-static-md/share/fmt/fmt-targets-debug.cmake|828
x86-windows-static-md/share/fmt/fmt-targets-release.cmake|824
x86-windows-static-md/share/fmt/fmt-targets.cmake|4715
x86-windows-static-md/share/fmt/usage|265
x86-windows-static-md/share/fmt/vcpkg.spdx.json|5558
x86-windows-static-md/share/fmt/vcpkg_abi_info.txt|1460
x86-windows-static-md/share/spdlog/copyright|1354
x86-windows-static-md/share/spdlog/spdlogConfig.cmake|1399
x86-windows-static-md/share/spdlog/spdlogConfigTargets-debug.cmake|864
x86-windows-static-md/share/spdlog/spdlogConfigTargets-release.cmake|860
x86-windows-static-md/share/spdlog/spdlogConfigTargets.cmake|4913
x86-windows-static-md/share/spdlog/spdlogConfigVersion.cmake|2830
x86-windows-static-md/share/spdlog/usage|286
x86-windows-static-md/share/spdlog/vcpkg.spdx.json|4212
x86-windows-static-md/share/spdlog/vcpkg_abi_info.txt|1404

'@ -split "`n")) {
        $line = $line.Trim()
        if ($line -eq '') { continue }
        $i = $line.LastIndexOf('|')
        if ($i -lt 1) { continue }
        $p = $line.Substring(0, $i)
        $s = [int64]$line.Substring($i + 1)
        $expected[$p.ToLowerInvariant()] = $s
    }

    Verify-All
    if ($script:missList.Count -eq 0 -and $script:wrongList.Count -eq 0) {
        Write-Log ('[OK] all ' + $expected.Count + ' source files verified')
    } else {
        $script:filesBroken = $true
        Show-Problems
        Try-Repair
        Verify-All
        Write-Log ''
        if ($script:missList.Count -eq 0 -and $script:wrongList.Count -eq 0) {
            Write-Log '[OK] re-check after repair: all files are correct now!' 'Green'
            $script:filesBroken = $false
        } else {
            Write-Log '[i] These files are still broken after the repair:'
            Show-Problems
        }
    }

    # ---------- 6) extra old files (info only) ----------
    Write-Log ''
    Write-Log '--- 6. Extra files (info) ---'
    $skipRoot = @('.vs', 'intermediates', 'release', 'debug', 'x64')
    $skipAny  = @('.git', '__pycache__')
    $stack = New-Object System.Collections.Stack
    $stack.Push('')
    $extras = 0
    while ($stack.Count -gt 0) {
        $rel = $stack.Pop()
        if ($rel -eq '') { $full = (Get-Location).Path } else { $full = Join-Path (Get-Location).Path $rel }
        $probe = $full
        if ($probe.Length -gt 240) { $probe = '\?\' + $probe }
        try {
            foreach ($fp in [IO.Directory]::GetFiles($probe)) {
                $n = [IO.Path]::GetFileName($fp)
                if ($rel -eq '') { $r = $n } else { $r = $rel + '/' + $n }
                $rl = $r.ToLowerInvariant()
                if ($expected.ContainsKey($rl)) { continue }
                if ($rl -eq 'check_source.ps1' -or $rl -eq 'check_result.txt') { continue }
                if ($rl.EndsWith('.zip') -or $rl -eq 'desktop.ini' -or $rl -eq 'thumbs.db') { continue }
                if ($extras -lt 15) { Write-Log ('  [i] extra: ' + $r) }
                $extras++
            }
            foreach ($dp in [IO.Directory]::GetDirectories($probe)) {
                $n = [IO.Path]::GetFileName($dp)
                if ($skipAny -contains $n) { continue }
                if ($rel -eq '') {
                    if ($skipRoot -contains $n) { continue }
                    $stack.Push($n)
                } else {
                    $stack.Push($rel + '\' + $n)
                }
            }
        } catch {
            Write-Log ('  [i] cannot read folder: ' + $rel)
        }
    }
    if ($extras -eq 0) { Write-Log '  [OK] no extra files' }
    else { Write-Log ('  [i] extra old files found: ' + $extras + ' - usually harmless junk') }

    # ---------- 7) verdict ----------
    Write-Log ''
    if ($script:broken -or $script:filesBroken) {
        Write-Log '=== RESULT: SOURCE NEEDS FIXES (see the [X] lines above) ===' 'Red'
        Write-Log 'If problems keep coming back: delete this folder, download lobo_edgebug_sound.zip'
        Write-Log ('again FROM THE CHAT (size must be 18669902 bytes), extract it with Windows')
        Write-Log '"Extract All" into a NEW empty folder, add the folder to antivirus exclusions,'
        Write-Log 'then run check_source.bat from that folder.'
    } else {
        Write-Log '=== RESULT: SOURCE IS OK ===' 'Green'
        Write-Log 'Now run build.bat (double-click it).'
    }
    Write-Log ''
    Write-Log ('Full log: ' + $logPath)
    Write-Log 'If you ask for help in the chat - send the TEXT of this log file.'

} catch {
    $script:broken = $true
    Write-Log ''
    Write-Log '[X] UNEXPECTED ERROR - the checker itself crashed:' 'Red'
    Write-Log ($_.ToString())
    Write-Log 'Send the file check_result.txt (in this folder) to the chat.'

} finally {
    Write-Log ''
    Read-Host 'Press Enter to exit'
}
