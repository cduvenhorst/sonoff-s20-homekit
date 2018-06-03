#!/usr/bin/env python

#   Copyright (C) 2018  Carsten Duvenhorst
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.

import random
import binascii
import sys
import struct
import imp

ALPHANUM = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ'
HEX_DIGITS = '0123456789ABCDEF'

rand = random.SystemRandom()

if sys.version_info.major == 2:  # pragma: no cover
    integer_types = (int, long)
else:  # pragma: no cover
    integer_types = (int,)

def base36(number):
    """Dumps an integer into a base36 string.
    :param number: the 10-based integer.
    :returns: the base36 string.
    """
    if not isinstance(number, integer_types):
        raise TypeError('number must be an integer')

    if number < 0:
        return '-' + base36(-number)

    value = ''

    while number != 0:
        number, index = divmod(number, len(ALPHANUM))
        value = ALPHANUM[index] + value

    return value or '0'


def generateSetupId():
    """
    Generates a random Setup ID for an ``Accessory`` or ``Bridge``.
    Used in QR codes and the setup hash.
    :return: 4 digit alphanumeric code.
    :rtype: str
    """
    return ''.join([
        rand.choice(ALPHANUM)
        for i in range(4)
    ])

def generateSetupCode():
    """
    Generates a random pincode.
    :return: setupCode in format ``xxx-xx-xxx``
    :rtype: bytearray
    """
    return '{}{}{}-{}{}-{}{}{}'.format(
        *(rand.randint(0, 9) for i in range(8))
    ).encode('ascii')

def xhmURI(setupId, setupCode):
        """Generates the X-HM:// uri (Setup Code URI)

        :rtype: str
        """
        buffer = bytearray(b'\x00\x00\x00\x00\x00\x00\x00\x00')

        version = 0
        reserved = 0
        category = 8 # switch
        transport = 2 # 2=IP, 4=BLE, 8=IP_WAC

        lowCategoryValue = 0

        lowCateroryValue = category & 3
        highCategoryValue = (category & 0xff) >> 2

        lowValue = lowCategoryValue

        lowValue <<= 4
        lowValue |= transport

        lowValue <<= 27
        lowValue |= int(setupCode.replace(b'-', b''), 10)

        highValue = version

        highValue <<= 4
        highValue |= reserved

        highValue <<= 6
        highValue |= highCategoryValue

        struct.pack_into('>L', buffer, 4, lowValue)
        struct.pack_into('>L', buffer, 0, highValue)

        encoded_payload = base36(
            struct.unpack_from('>L', buffer, 4)[0] +
            (struct.unpack_from('>L', buffer, 0)[0] * (1 << 32))).upper()

        encoded_payload = encoded_payload.rjust(9, '0')

        return 'X-HM://' + encoded_payload + setupId

def main():

    setupCode = generateSetupCode()
    setupId = generateSetupId()
    setupURI = xhmURI(setupId, setupCode)

    configPath = sys.argv[1]
    qrcodePath = sys.argv[2]

    with open(configPath, "w") as config:
        config.write("// Generated pairing information. Use \"make homekitSettings\" to generate.\n\n")
        config.write("#define SETUP_CODE \"{0}\"\n".format(setupCode))
        config.write("#define SETUP_ID \"{0}\"\n".format(setupId))
        config.write("// URI is \"{0}\"\n\n".format(xhmURI(setupId, setupCode)))

    try:
        import png
        import pyqrcode
        from pyqrcode import QRCode

        QRCode(setupURI).png(qrcodePath, scale=3)

    except ImportError, err:
            print 'Can not find a required module for QR-Code generation:', err
            print 'Install it using \"pip install pyqrcode pypng\".\n'

if __name__ == '__main__':
    main()
