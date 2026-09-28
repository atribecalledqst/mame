// license:BSD-3-Clause
// copyright-holders:atribecalledqst
// thanks-to:azya
/*******************************************************************************

  Bandai Tamagotchi generation 1 hardware
  * PCB label TMG-M1
  * Seiko Epson E0C6S48 MCU under epoxy

*******************************************************************************/

#include "emu.h"

#include "cpu/e0c6200/e0c6s48.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "yasashii.lh"


namespace {

class yasashiig1_state : public driver_device
{
public:
	yasashiig1_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_out_x(*this, "%u.%u", 0U, 0U)
	{ }

	void yasashii(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(input_changed);

private:
	void yasashii_palette(palette_device &palette) const;
	E0C6S48_PIXEL_UPDATE(pixel_update);

	required_device<e0c6s48_device_2> m_maincpu;
	output_finder<16, 40> m_out_x;
};

/*******************************************************************************
    Video
*******************************************************************************/

E0C6S48_PIXEL_UPDATE(yasashiig1_state::pixel_update)
{
	// 16 COM(common) pins, 40 SEG(segment) pins from MCU,
	// 32x16 LCD screen:
	static const int seg2x[0x28] =
	{
	  0, 1, 2, 3, 4, 5, 6, 7,
	  35, 8, 9,10,11,12,13,14,
	  15,34,33,32,31,30,29,28,
	  27,26,25,24,36,23,22,21,
	  20,19,18,17,16,37,38,39
	};

	int y = com, x = seg2x[seg];
	if (cliprect.contains(x, y))
		bitmap.pix(y, x) = state;

	// 2 rows of indicators:
	// above screen: 0:check, 1:food, 2:play, 3:toilet
	// under screen: 4:discipline, 5:medicine, 6:lights, 7:attention
	// they are on pin SEG8(x=35) + COM0-3, pin SEG28(x=36) + COM12-15
  // TODO? The yasashii does not have the usual rows of indicators, but it
  // does flash lights on the 6 care buttons. So it might be nice to
  // render the flashing buttons somehow.

	// output to y.x
	m_out_x[y][x] = state;
}

void yasashiig1_state::yasashii_palette(palette_device &palette) const
{
	palette.set_pen_color(0, rgb_t(0xf1, 0xf0, 0xf9)); // background
	palette.set_pen_color(1, rgb_t(0x3c, 0x38, 0x38)); // lcd pixel
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

INPUT_CHANGED_MEMBER(yasashiig1_state::input_changed)
{
	// inputs are hooked up backwards here, because MCU input
	// ports are all tied to its interrupt controller
	int line = param;
	int state = newval ? ASSERT_LINE : CLEAR_LINE;
	m_maincpu->set_input_line(line, state);
}

static INPUT_PORTS_START( yasashii )
	PORT_START("K0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(yasashiig1_state::input_changed), E0C6S48_LINE_K00)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(yasashiig1_state::input_changed), E0C6S48_LINE_K01)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(yasashiig1_state::input_changed), E0C6S48_LINE_K02)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(yasashiig1_state::input_changed), E0C6S48_LINE_K03)
  PORT_START("K1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(yasashiig1_state::input_changed), E0C6S48_LINE_K10)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(yasashiig1_state::input_changed), E0C6S48_LINE_K11)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(yasashiig1_state::input_changed), E0C6S48_LINE_K12)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON8 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(yasashiig1_state::input_changed), E0C6S48_LINE_K13)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void yasashiig1_state::yasashii(machine_config &config)
{
	/* basic machine hardware */
	E0C6S48_2(config, m_maincpu, 32.768_kHz_XTAL);
	m_maincpu->set_pixel_update_cb(FUNC(yasashiig1_state::pixel_update));
	// m_maincpu->write_r<4>().set("speaker", FUNC(speaker_sound_device::level_w)).bit(3);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen").set_lcd());
	screen.set_refresh_hz(32.768_kHz_XTAL/1024);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(40, 16);
	screen.set_visarea(0, 32-1, 0, 16-1);
	screen.set_screen_update("maincpu", FUNC(e0c6s48_device_2::screen_update));
	screen.set_palette("palette");
	config.set_default_layout(layout_yasashii);

	PALETTE(config, "palette", FUNC(yasashiig1_state::yasashii_palette), 2);

	/* sound hardware */
  // I need to troubleshoot other stuff and I'm not sure you can actually mute it in software.
	// SPEAKER(config, "mono").front_center();
	// SPEAKER_SOUND(config, "speaker").add_route(ALL_OUTPUTS, "mono", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( yasashii )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "yasashii.bin", 0x0000, 0x4000, CRC(0549477a) SHA1(dc713b7eb4cbc5d9e921fe3bd97662c72bc68534) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS         INIT        COMPANY,  FULLNAME,           FLAGS
CONS( 1998, yasashii, 0,      0,      yasashii,    yasashii,  yasashiig1_state, empty_init, "Bandai", "Tamagotchi Yasashii (JP)", MACHINE_SUPPORTS_SAVE )
