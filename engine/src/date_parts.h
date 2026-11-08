#pragma once

struct DATE_PARTS
{
    int year = 1970;
    int month = 1;
    int day = 1;
    int hour = 0;
    int minute = 0;
    int second = 0;

    bool operator==(const DATE_PARTS &other) const = default;
};
