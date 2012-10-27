"""
License: GPL
Author: Valerij Klein <vklein@users.sourceforge.net>
$Id$

"""
__version__ = "$Revision$"[11:-2]
__date__ = "$Date$"[7:-2]

from misc import ClbConst

try: 
    import gettext
    import sys
    gettext.bindtextdomain(ClbConst.APPNAME, ClbConst.LOCALEPATH)
    gettext.textdomain(ClbConst.APPNAME)
    _ = gettext.gettext

except:
    def _(str):
        """pass given string as-is"""
        return str

if __name__ == '__main__':
    pass

