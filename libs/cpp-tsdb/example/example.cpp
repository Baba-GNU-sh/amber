#include <tsdb/tsdb.hpp>
#include <iostream>
#include <boost/timer/timer.hpp>

int main(void)
{
    SparseTimeSeries ts(true);
    Time t = 0;

    while (true)
    {
        const Time SAMPLES = 1'000;

        std::vector<Sample> samples(SAMPLES);
        for (Time i = 0; i < SAMPLES; ++i)
        {
            samples[i].time = t++;
            samples[i].value = i;
        }

        {
            boost::timer::auto_cpu_timer timer;
            ts.push_some(samples);
        }

        {
            boost::timer::auto_cpu_timer timer;
            auto vec = ts.mean(t - 1'000, 40, 1'000);
        }

        std::cout << "Nsamples = " << ts.span().second << '\n';
    }
}