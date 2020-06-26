// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

    Green Beret

***************************************************************************/
#ifndef MAME_INCLUDES_GBERET_H
#define MAME_INCLUDES_GBERET_H

#pragma once

#include "machine/timer.h"
#include "sound/sn76496.h"
#include "emupal.h"
#include "tilemap.h"

class gberet_state : public driver_device
{
public:
	gberet_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram2(*this, "spriteram2"),
		m_spriteram(*this, "spriteram"),
		m_scrollram(*this, "scrollram"),
		m_soundlatch(*this, "soundlatch"),
		m_sn(*this, "snsnd") ,
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void gberetb(machine_config &config);
	void mrgoemon(machine_config &config);
	void gberet(machine_config &config);

	void init_mrgoemon();

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	optional_shared_ptr<uint8_t> m_spriteram2;
	required_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_scrollram;
	optional_shared_ptr<uint8_t> m_soundlatch;

	/* devices */
	required_device<sn76489a_device> m_sn;

	/* video-related */
	tilemap_t * m_bg_tilemap;
	uint8_t       m_spritebank;

	/* misc */
	uint8_t       m_interrupt_mask;
	uint8_t       m_interrupt_ticks;
	void gberet_coin_counter_w(uint8_t data);
	void mrgoemon_coin_counter_w(uint8_t data);
	void gberet_flipscreen_w(uint8_t data);
	void gberet_sound_w(uint8_t data);
	void gberetb_flipscreen_w(uint8_t data);
	uint8_t gberetb_irq_ack_r();
	void gberetb_nmi_ack_w(uint8_t data);
	void gberet_videoram_w(offs_t offset, uint8_t data);
	void gberet_colorram_w(offs_t offset, uint8_t data);
	void gberet_scroll_w(offs_t offset, uint8_t data);
	void gberet_sprite_bank_w(uint8_t data);
	void gberetb_scroll_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	DECLARE_MACHINE_START(gberet);
	DECLARE_MACHINE_RESET(gberet);
	DECLARE_VIDEO_START(gberet);
	void gberet_palette(palette_device &palette) const;
	uint32_t screen_update_gberet(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_gberetb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(gberet_interrupt_tick);
	void gberet_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void gberetb_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	void gberet_map(address_map &map);
	void gberetb_map(address_map &map);
	void mrgoemon_map(address_map &map);
};

#endif // MAME_INCLUDES_GBERET_H
