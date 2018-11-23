// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Skeleton driver for Falco TS-series terminals.

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
//#include "bus/rs232/rs232.h"
#include "machine/nvram.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "video/mc6845.h"
#include "screen.h"

class falcots_state : public driver_device
{
public:
	falcots_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_crtc(*this, "crtc")
		, m_vram(*this, "vram")
		, m_chargen(*this, "chargen")
		, m_keys(*this, "KEY%X", 0U)
	{
	}

	void ts1(machine_config &config);
	void ts2624(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	MC6845_UPDATE_ROW(update_row);

	DECLARE_WRITE_LINE_MEMBER(hsync_w);

	u8 key_status_r();
	void key_scan_w(u8 data);
	void vram0_w(u8 data);
	void vram1_w(u8 data);

	void mem_map(address_map &map);
	void io_map(address_map &map);
	void ts1_io_map(address_map &map);

	required_device<z80_device> m_maincpu;
	required_device<mc6845_device> m_crtc;
	required_shared_ptr<u8> m_vram;
	required_region_ptr<u8> m_chargen;
	required_ioport_array<12> m_keys;

	u8 m_key_scan;
};

void falcots_state::machine_start()
{
	m_key_scan = 0;
	save_item(NAME(m_key_scan));
}

MC6845_UPDATE_ROW(falcots_state::update_row)
{
}

u8 falcots_state::key_status_r()
{
	u8 status = m_crtc->vsync_r() ? 0x00 : 0x02;

	u8 i = (m_key_scan & 0x70) >> 4;
	u8 j = m_key_scan & 0x0f;
	if (j < 12 && BIT(m_keys[j]->read(), i) == BIT(m_key_scan, 7))
		status |= 0x01;

	return status;
}

void falcots_state::key_scan_w(u8 data)
{
	// TODO: this is a vastly oversimplification of the keyboard interface
	// (actual interface uses various shift registers driven by an unknown clock)
	m_key_scan = data;
}

void falcots_state::vram0_w(u8 data)
{
}

void falcots_state::vram1_w(u8 data)
{
}

void falcots_state::mem_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0x87ff).ram().share("nvram");
	map(0xa000, 0xbfff).ram(); // 4x HM6116P-3
	map(0xc000, 0xffff).ram().share("vram"); // 8x AM9016EPC (4116)
}

void falcots_state::ts1_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xe1, 0xe1).w(FUNC(falcots_state::key_scan_w));
	map(0xe8, 0xe8).r(FUNC(falcots_state::key_status_r));
	map(0xf0, 0xf0).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0xf1, 0xf1).w(m_crtc, FUNC(mc6845_device::register_w));
}

void falcots_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xe0, 0xe0).r(FUNC(falcots_state::key_status_r));
	map(0xe1, 0xe1).w(FUNC(falcots_state::key_scan_w));
	map(0xe2, 0xe2).w(FUNC(falcots_state::vram0_w));
	map(0xe4, 0xe4).w(FUNC(falcots_state::vram1_w));
	map(0xe8, 0xe8).w(m_crtc, FUNC(mc6845_device::address_w));
	map(0xe9, 0xe9).w(m_crtc, FUNC(mc6845_device::register_w));
	map(0xf0, 0xf3).rw("dart", FUNC(z80dart_device::ba_cd_r), FUNC(z80dart_device::ba_cd_w));
	map(0xf8, 0xfb).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
}

static INPUT_PORTS_START(ts2624)
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Aids")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x1b) PORT_NAME("Escape") PORT_CODE(KEYCODE_ESC)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('1') PORT_CHAR('!') PORT_CODE(KEYCODE_1)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(0x09) PORT_NAME("Tab  Back Tab") PORT_CODE(KEYCODE_TAB)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('q') PORT_CHAR('Q') PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Control") PORT_CHAR(UCHAR_SHIFT_2) PORT_CODE(KEYCODE_LCONTROL)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Shift Caps") PORT_CHAR(UCHAR_MAMEKEY(CAPSLOCK)) PORT_CODE(KEYCODE_CAPSLOCK)

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("User Keys")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Mode")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('2') PORT_CHAR('@') PORT_CODE(KEYCODE_2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('3') PORT_CHAR('#') PORT_CODE(KEYCODE_3)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('w') PORT_CHAR('W') PORT_CODE(KEYCODE_W)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('a') PORT_CHAR('A') PORT_CODE(KEYCODE_A)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('z') PORT_CHAR('Z') PORT_CODE(KEYCODE_Z)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Left Shift") PORT_CHAR(UCHAR_SHIFT_1) PORT_CODE(KEYCODE_LSHIFT)

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F2)) PORT_CODE(KEYCODE_F2)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F1)) PORT_CODE(KEYCODE_F1)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('4') PORT_CHAR('$') PORT_CODE(KEYCODE_4)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('e') PORT_CHAR('E') PORT_CODE(KEYCODE_E)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('s') PORT_CHAR('S') PORT_CODE(KEYCODE_S)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('d') PORT_CHAR('D') PORT_CODE(KEYCODE_D)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('x') PORT_CHAR('X') PORT_CODE(KEYCODE_X)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Function") PORT_CHAR(UCHAR_MAMEKEY(LALT)) PORT_CODE(KEYCODE_LALT)

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F3)) PORT_CODE(KEYCODE_F3)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('5') PORT_CHAR('%') PORT_CODE(KEYCODE_5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('r') PORT_CHAR('R') PORT_CODE(KEYCODE_R)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('t') PORT_CHAR('T') PORT_CODE(KEYCODE_T)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('f') PORT_CHAR('F') PORT_CODE(KEYCODE_F)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('g') PORT_CHAR('G') PORT_CODE(KEYCODE_G)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('v') PORT_CHAR('V') PORT_CODE(KEYCODE_V)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('c') PORT_CHAR('C') PORT_CODE(KEYCODE_C)

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F4)) PORT_CODE(KEYCODE_F4)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F5)) PORT_CODE(KEYCODE_F5)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('6') PORT_CHAR('^') PORT_CODE(KEYCODE_6)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('7') PORT_CHAR('&') PORT_CODE(KEYCODE_7)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('y') PORT_CHAR('Y') PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('h') PORT_CHAR('H') PORT_CODE(KEYCODE_H)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('b') PORT_CHAR('B') PORT_CODE(KEYCODE_B)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(' ') PORT_CODE(KEYCODE_SPACE)

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F6)) PORT_CODE(KEYCODE_F6)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F7)) PORT_CODE(KEYCODE_F7)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('8') PORT_CHAR('*') PORT_CODE(KEYCODE_8)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('i') PORT_CHAR('I') PORT_CODE(KEYCODE_I)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('u') PORT_CHAR('U') PORT_CODE(KEYCODE_U)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('j') PORT_CHAR('J') PORT_CODE(KEYCODE_J)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('m') PORT_CHAR('M') PORT_CODE(KEYCODE_M)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('n') PORT_CHAR('N') PORT_CODE(KEYCODE_N)

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(F8)) PORT_CODE(KEYCODE_F8)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('0') PORT_CHAR(')') PORT_CODE(KEYCODE_0)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('9') PORT_CHAR('(') PORT_CODE(KEYCODE_9)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('o') PORT_CHAR('O') PORT_CODE(KEYCODE_O)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('k') PORT_CHAR('K') PORT_CODE(KEYCODE_K)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('l') PORT_CHAR('L') PORT_CODE(KEYCODE_L)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('.') PORT_CHAR('>') PORT_CODE(KEYCODE_STOP)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(',') PORT_CHAR('<') PORT_CODE(KEYCODE_COMMA)

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ins/Del Line")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Ins/Del Char")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('-') PORT_CHAR('_') PORT_CODE(KEYCODE_MINUS)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('[') PORT_CHAR('{') PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('p') PORT_CHAR('P') PORT_CODE(KEYCODE_P)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(';') PORT_CHAR(':') PORT_CODE(KEYCODE_COLON)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('/') PORT_CHAR('?') PORT_CODE(KEYCODE_SLASH)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Back Space") PORT_CHAR(0x08) PORT_CODE(KEYCODE_RALT)

	PORT_START("KEY8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Line/Dsp Clear")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('=') PORT_CHAR('+') PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('`') PORT_CHAR('~') PORT_CODE(KEYCODE_TILDE)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Delete") PORT_CHAR(0x7f) PORT_CODE(KEYCODE_DEL)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(']') PORT_CHAR('}') PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\\') PORT_CHAR('|') PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR('\'') PORT_CHAR('"') PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Right Shift") PORT_CODE(KEYCODE_RSHIFT)

	PORT_START("KEY9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(HOME)) PORT_CODE(KEYCODE_HOME)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Back Space (Break)") PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(7_PAD)) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 4  \xe2\x86\x90") PORT_CHAR(UCHAR_MAMEKEY(4_PAD)) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Return") PORT_CHAR(0x0d) PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(1_PAD)) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Enter") PORT_CHAR(UCHAR_MAMEKEY(ENTER_PAD)) PORT_CODE(KEYCODE_RCONTROL)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(0_PAD)) PORT_CODE(KEYCODE_0_PAD)

	PORT_START("KEYA")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xe2\x86\x90  Prev Page") PORT_CHAR(UCHAR_MAMEKEY(LEFT)) PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xe2\x86\x91  Scroll Up") PORT_CHAR(UCHAR_MAMEKEY(UP)) PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 8  \xe2\x86\x91 (Brite)") PORT_CHAR(UCHAR_MAMEKEY(8_PAD)) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 5  Home") PORT_CHAR(UCHAR_MAMEKEY(5_PAD)) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 2  \xe2\x86\x93 (Dim)") PORT_CHAR(UCHAR_MAMEKEY(2_PAD)) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(3_PAD)) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD)) PORT_CODE(KEYCODE_DEL_PAD)

	PORT_START("KEYB")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xe2\x86\x93  Scroll Down") PORT_CHAR(UCHAR_MAMEKEY(DOWN)) PORT_CODE(KEYCODE_DOWN)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("\xe2\x86\x92  Next Page") PORT_CHAR(UCHAR_MAMEKEY(RIGHT)) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(MINUS_PAD)) PORT_CODE(KEYCODE_MINUS_PAD)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CHAR(UCHAR_MAMEKEY(9_PAD)) PORT_CODE(KEYCODE_9_PAD)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad 6  \xe2\x86\x92") PORT_CHAR(UCHAR_MAMEKEY(6_PAD)) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad Back Tab") PORT_CODE(KEYCODE_PLUS_PAD)
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_NAME("Keypad Forward Tab") PORT_CHAR(UCHAR_MAMEKEY(TAB_PAD)) PORT_CODE(KEYCODE_ENTER_PAD)
INPUT_PORTS_END

static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ "dart" },
	{ nullptr }
};

void falcots_state::ts2624(machine_config &config)
{
	Z80(config, m_maincpu, 14.7456_MHz_XTAL / 4); // Z8400AB1
	m_maincpu->set_addrmap(AS_PROGRAM, &falcots_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &falcots_state::io_map);
	m_maincpu->set_daisy_config(daisy_chain);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0); // HM6116LP-4 + battery

	z80ctc_device &ctc(Z80CTC(config, "ctc", 14.7456_MHz_XTAL / 4)); // Z8430AB1
	ctc.set_clk<0>(14.7456_MHz_XTAL / 16);
	ctc.set_clk<1>(14.7456_MHz_XTAL / 16);
	ctc.zc_callback<0>().set("dart", FUNC(z80dart_device::rxca_w));
	ctc.zc_callback<1>().set("dart", FUNC(z80dart_device::txca_w));
	ctc.zc_callback<2>().set("dart", FUNC(z80dart_device::rxtxcb_w));
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	z80dart_device &dart(Z80DART(config, "dart", 14.7456_MHz_XTAL / 4)); // Z8470AB1
	dart.out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(14.7456_MHz_XTAL, 768, 0, 640, 320, 0, 286);
	//screen.set_raw(23.9616_MHz_XTAL, 1248, 0, 1056, 320, 0, 286);
	screen.set_screen_update("crtc", FUNC(mc6845_device::screen_update));

	MC6845(config, m_crtc, 14.7456_MHz_XTAL / 16);
	m_crtc->set_screen("screen");
	m_crtc->set_show_border_area(false);
	m_crtc->set_char_width(16);
	m_crtc->set_update_row_callback(FUNC(falcots_state::update_row), this);
}

void falcots_state::ts1(machine_config &config)
{
	ts2624(config);

	m_maincpu->set_addrmap(AS_IO, &falcots_state::ts1_io_map);

	m_crtc->set_clock(15'206'400 / 8);
	m_crtc->set_char_width(8);

	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_raw(15'206'400, 792, 0, 640, 320, 0, 300);
}

ROM_START(ts1)
	ROM_REGION(0x8000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("v2_13_x.d9",  0x0000, 0x1000, CRC(420e1ecd) SHA1(748e3733858ba813b9d72dfe018ba4f918d8c0db)) // Chip Type: 2732
	ROM_LOAD("v2_13_0.d10", 0x1000, 0x1000, CRC(228e7321) SHA1(43e0d04c58ee7c71f5603222bf0aaaf7979d67a3)) // Chip Type: 2732

	ROM_REGION(0x2000, "chargen", ROMREGION_ERASEFF)
	ROM_LOAD("crom003.f4", 0x0000, 0x0800, CRC(557c8e0b) SHA1(b028f526bd92f957ee6242a7e0e6e0f16b0880a8)) // Chip Type: EA8316E517
ROM_END

ROM_START(ts2624)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("1.bin", 0x0000, 0x2000, CRC(14fb80aa) SHA1(93bf0d39f3e4bf092b6cd850f95ee6cbd322ad13))
	ROM_LOAD("2.bin", 0x2000, 0x2000, CRC(d4c74a06) SHA1(291357a296c45fccdbe8e395ea170d847a3a6f03))
	ROM_LOAD("3.bin", 0x4000, 0x2000, CRC(90d0d04b) SHA1(099d6741091b3abbe4187c8278e2c7ebe151531c))
	ROM_LOAD("4.bin", 0x6000, 0x2000, CRC(b0c59ec8) SHA1(099f6d6a7594e177bc668fd19fa19c3f0f4ab38e))

	ROM_REGION(0x2000, "chargen", 0)
	ROM_LOAD("chr.bin", 0x0000, 0x2000, CRC(38569fe2) SHA1(c666c596bb6326e4f41ccfd91154bcfd75f5c0a3))

	ROM_REGION(0x60, "proms", 0)
	ROM_LOAD("msel64b.9c", 0x00, 0x20, NO_DUMP) // 74S288 or equivalent
	ROM_LOAD("prom.13d",   0x20, 0x20, NO_DUMP) // 74S288 or equivalent
	ROM_LOAD("prom.12f",   0x20, 0x20, NO_DUMP) // 74S288 or equivalent
ROM_END

COMP(1980, ts1,    0, 0, ts1,    ts2624, falcots_state, empty_init, "Falco Data Products", "TS-1 (v2.13.0)", MACHINE_IS_SKELETON)
COMP(1982, ts2624, 0, 0, ts2624, ts2624, falcots_state, empty_init, "Falco Data Products", "TS-2624", MACHINE_IS_SKELETON)