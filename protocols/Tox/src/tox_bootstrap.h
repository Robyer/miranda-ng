#ifndef _TOX_BOOTSTRAP_H_
#define _TOX_BOOTSTRAP_H_

struct bootstrap_node {
	char *address;
	uint16_t port;
	uint8_t key[32];
} bootstrap_nodes[] = {
	{
		"192.254.75.98",
		33445,
		{
			0x95, 0x1C, 0x88, 0xB7, 0xE7, 0x5C, 0x86, 0x74, 0x18, 0xAC, 0xDB, 0x5D, 0x27, 0x38, 0x21, 0x37,
			0x2B, 0xB5, 0xBD, 0x65, 0x27, 0x40, 0xBC, 0xDF, 0x62, 0x3A, 0x4F, 0xA2, 0x93, 0xE7, 0x5D, 0x2F
		}
	},

	{
		"31.7.57.236",
		443,
		{
			0x95, 0x1C, 0x88, 0xB7, 0xE7, 0x5C, 0x86, 0x74, 0x18, 0xAC, 0xDB, 0x5D, 0x27, 0x38, 0x21, 0x37,
			0x2B, 0xB5, 0xBD, 0x65, 0x27, 0x40, 0xBC, 0xDF, 0x62, 0x3A, 0x4F, 0xA2, 0x93, 0xE7, 0x5D, 0x2F
		}
	},

	{
		"107.161.17.51",
		33445,
		{
			0x7B, 0xE3, 0x95, 0x1B, 0x97, 0xCA, 0x4B, 0x9E, 0xCD, 0xDA, 0x76, 0x8E, 0x8C, 0x52, 0xBA, 0x19,
			0xE9, 0xE2, 0x69, 0x0A, 0xB5, 0x84, 0x78, 0x7B, 0xF4, 0xC9, 0x0E, 0x04, 0xDB, 0xB7, 0x51, 0x11
		}
	},

	{
		"144.76.60.215",
		33445,
		{
			0x04, 0x11, 0x9E, 0x83, 0x5D, 0xF3, 0xE7, 0x8B, 0xAC, 0xF0, 0xF8, 0x42, 0x35, 0xB3, 0x00, 0x54,
			0x6A, 0xF8, 0xB9, 0x36, 0xF0, 0x35, 0x18, 0x5E, 0x2A, 0x8E, 0x9E, 0x0A, 0x67, 0xC8, 0x92, 0x4F
		}
	},

	{
		"23.226.230.47",
		33445,
		{
			0xA0, 0x91, 0x62, 0xD6, 0x86, 0x18, 0xE7, 0x42, 0xFF, 0xBC, 0xA1, 0xC2, 0xC7, 0x03, 0x85, 0xE6,
			0x67, 0x96, 0x04, 0xB2, 0xD8, 0x0E, 0xA6, 0xE8, 0x4A, 0xD0, 0x99, 0x6A, 0x1A, 0xC8, 0xA0, 0x74
		}
	},

	{
		"37.59.102.176",
		33445,
		{
			0xB9, 0x8A, 0x2C, 0xEA, 0xA6, 0xC6, 0xA2, 0xFA, 0xDC, 0x2C, 0x36, 0x32, 0xD2, 0x84, 0x31, 0x8B,
			0x60, 0xFE, 0x53, 0x75, 0xCC, 0xB4, 0x1E, 0xFA, 0x08, 0x1A, 0xB6, 0x7F, 0x50, 0x0C, 0x1B, 0x0B
		}
	},

	{
		"37.187.46.132",
		33445,
		{
			0x2A, 0x4B, 0x50, 0xD1, 0xD5, 0x25, 0xDA, 0x2E, 0x66, 0x95, 0x92, 0xA2, 0x0C, 0x32, 0x7B, 0x5F,
			0xAD, 0x6C, 0x7E, 0x59, 0x62, 0xDC, 0x69, 0x29, 0x6F, 0x9F, 0xEC, 0x77, 0xC4, 0x43, 0x6E, 0x4E
		}
	},

	{
		"178.21.112.187",
		33445,
		{
			0x4B, 0x2C, 0x19, 0xE9, 0x24, 0x97, 0x2C, 0xB9, 0xB5, 0x77, 0x32, 0xFB, 0x17, 0x2F, 0x8A, 0x86,
			0x04, 0xDE, 0x13, 0xEE, 0xDA, 0x2A, 0x62, 0x34, 0xE3, 0x48, 0x98, 0x33, 0x44, 0xB2, 0x30, 0x57
		}
	},

	{
		"192.210.149.121",
		33445,
		{
			0xF4, 0x04, 0xAB, 0xAA, 0x1C, 0x99, 0xA9, 0xD3, 0x7D, 0x61, 0xAB, 0x54, 0x89, 0x8F, 0x56, 0x79,
			0x3E, 0x1D, 0xEF, 0x8B, 0xD4, 0x6B, 0x10, 0x38, 0xB9, 0xD8, 0x22, 0xE8, 0x46, 0x0F, 0xAB, 0x67
		}
	},

	{
		"54.199.139.199",
		33445,
		{
			0x7F, 0x9C, 0x31, 0xFE, 0x85, 0x0E, 0x97, 0xCE, 0xFD, 0x4C, 0x45, 0x91, 0xDF, 0x93, 0xFC, 0x75,
			0x7C, 0x7C, 0x12, 0x54, 0x9D, 0xDD, 0x55, 0xF8, 0xEE, 0xAE, 0xCC, 0x34, 0xFE, 0x76, 0xC0, 0x29
		}
	},

	{
		"63.165.243.15",
		443,
		{
			0x8C, 0xD0, 0x87, 0xE3, 0x1C, 0x67, 0x56, 0x81, 0x03, 0xE8, 0xC2, 0xA2, 0x86, 0x53, 0x33, 0x7E,
			0x90, 0xE6, 0xB8, 0xED, 0xA0, 0xD7, 0x65, 0xD5, 0x7C, 0x6B, 0x51, 0x72, 0xB4, 0xF1, 0xF0, 0x4C
		}
	},

	{
		"195.154.119.113",
		33445,
		{
			0xE3, 0x98, 0xA6, 0x96, 0x46, 0xB8, 0xCE, 0xAC, 0xA9, 0xF0, 0xB8, 0x4F, 0x55, 0x37, 0x26, 0xC1,
			0xC4, 0x92, 0x70, 0x55, 0x8C, 0x57, 0xDF, 0x5F, 0x3C, 0x36, 0x8F, 0x05, 0xA7, 0xD7, 0x13, 0x54
		}
	},
};

#endif //_TOX_BOOTSTRAP_H_