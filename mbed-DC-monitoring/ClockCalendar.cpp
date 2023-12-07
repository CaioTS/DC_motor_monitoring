#include "ClockCalendar.h"

Clock::Clock(int h, int m, int s, int pm){
    hr = h;
    min = m;
    sec = s;
    is_pm = pm;
}
void Clock::setClock(int h,int m, int s , int pm){
    hr = h;
    min = m;
    sec = s;
    is_pm = pm;
}

void Clock::readClock(int& h,int& m, int& s , int& pm){
    h = hr;
    m = min;
    s = sec;
    pm = is_pm;
}

void Clock::advance(){
    sec++;
    if (sec == 60)
    {
        sec = 0;
        min++; 
        if (min == 60){
            min = 0;
            hr++;
            if (hr ==12){
                hr = 0;
                if (is_pm == 0)is_pm = 1;
                else is_pm = 0;
            }
        }
    }    
}

Calendar::Calendar(int m,int d, int y){
    mo = m;
    day=  d;
    yr = y;
}

void Calendar::setCalendar(int m,int d, int y){
    mo = m;
    day = d;
    yr = y;
}

void Calendar::readCalendar(int& m,int& d, int& y){
    m = mo;
    d = day;
    y = yr;
}

void Calendar::advance(){
    day++;
    int dias_no_mes;
    bool bissexto;

    if ((yr)%4 == 0)bissexto = true;
    else bissexto = false;
    if (mo == 1 || mo == 3 || mo == 5 || mo == 7 || mo == 8 || mo == 10 || mo == 12)dias_no_mes = 31;
    else if (mo ==2)
    {
        if (bissexto){dias_no_mes = 29;}
        else dias_no_mes = 28;
    }
    else dias_no_mes = 30;

    if (day == dias_no_mes+1){
        mo++;
        day = 1;
        if (mo == 13){
            yr++;
            mo = 1;
        }
    }
}


ClockCalendar::ClockCalendar (int mt, int d, int y, int h, int m, int s, int pm) 
: Clock (h, m, s, pm), Calendar (mt, d, y)
{}

void ClockCalendar::advance(){ // avançar o calendário, caso o clock
    int wasPm = is_pm; // mude de AM para PM.
    Clock::advance();
    if (wasPm && !is_pm)
    Calendar::advance();
}
