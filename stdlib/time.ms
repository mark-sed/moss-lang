d"""
This module provides time related resources.
"""

class StructTime {
    fun StructTime(tm_year:Int, tm_mon:Int, tm_mday:Int, tm_hour:Int,
                   tm_min:Int, tm_sec:Int, tm_wday:Int, tm_yday:Int,
                   tm_isdst:Int, tm_zone:String="", tm_gmtoff:[Int,NilType]=nil) {
        this.tm_year = tm_year
        this.tm_mon = tm_mon
        this.tm_mday = tm_mday
        this.tm_hour = tm_hour
        this.tm_min = tm_min
        this.tm_sec = tm_sec
        this.tm_wday = tm_wday
        this.tm_yday = tm_yday
        this.tm_isdst = tm_isdst
        this.tm_zone = tm_zone
        this.tm_gmtoff = tm_gmtoff
    }

    fun __String() {
        return f"time.StructTime(tm_year={this.tm_year}, tm_mon={this.tm_mon}, \
tm_mday={this.tm_mday}, tm_hour={this.tm_hour}, tm_min={this.tm_min}, \
tm_sec={this.tm_sec}, tm_wday={this.tm_wday}, tm_yday={this.tm_yday}, \
tm_isdst={this.tm_isdst}, tm_zone=\"{this.tm_zone}\", tm_gmtoff={this.tm_gmtoff})"
    }
}

@internal
fun time() {}

@internal
fun localtime(secs:[Int,Float,NilType]=nil) {}

@internal
fun strftime(format, t:[StructTime,NilType]=nil) {}