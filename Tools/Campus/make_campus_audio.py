"""Original, quiet ambience for the NightSchool dormitory and hall."""

import array
from math import cos, exp, pi, sin, tau
from pathlib import Path
from random import Random
import wave


RATE = 24000
HERE = Path(__file__).resolve().parent


def write_wave(name, frames, channels):
    pcm = array.array('h', (round(max(-.98, min(.98, sample)) * 32767) for frame in frames for sample in frame))
    with wave.open(str(HERE / f'{name}.wav'), 'wb') as out:
        out.setnchannels(channels)
        out.setsampwidth(2)
        out.setframerate(RATE)
        out.writeframes(pcm.tobytes())


def air_cycle(seconds, seed):
    rng = Random(seed)
    wanted = [rng.uniform(-1, 1) for _ in range(seconds * RATE)]
    wind, low, high = [], 0., 0.
    for v in wanted[-RATE:] + wanted:
        low += .020 * (v - low)
        high += .065 * (v - high)
        wind.append(4.1 * low + 1.3 * (high - low))
    return wind[RATE:]


def corridor_air():
    air_left, air_right = air_cycle(24, 541), air_cycle(24, 419)
    for i, (left, right) in enumerate(zip(air_left, air_right, strict=True)):
        t = i / RATE
        envelope = .50 + .19 * cos(tau * t / 24) + .11 * sin(tau * 3 * t / 24)
        flutter = sin(tau * 8 * t / 24)
        sub = .016 * sin(tau * 42 * t) + .010 * sin(tau * (61 * t + .75 * flutter))
        structural = .014 * sin(tau * (95.5 * t + .24 * sin(tau * t / 24))) * (.5 + .5 * flutter)
        yield (.15 * envelope * left + sub + structural, .15 * envelope * right + sub + structural)


def ballast():
    air = air_cycle(4, 105)
    for i, noise in enumerate(air):
        t = i / RATE
        drift = .76 + .12 * sin(tau * .75 * t) + .095 * sin(tau * 1.25 * t)
        tone = .05 * sin(tau * 100 * t) + .042 * sin(tau * 150 * t) + .021 * sin(tau * 450 * t)
        yield ((tone + noise * .046) * drift,)


def vent_knock():
    rng = Random(737)
    noise = 0.
    for i in range(3 * RATE):
        t = i / RATE
        noise += .13 * (rng.uniform(-1, 1) - noise)
        sample = 0.
        for strike, gain in ((.08, .75), (.44, 1.), (.68, .35)):
            d = t - strike
            if d >= 0:
                pulse = min(1., 900 * d)
                sample += gain * pulse * (
                    .22 * exp(-12*d) * sin(tau * 121 * d)
                    + .115 * exp(-9*d) * sin(tau * 273 * d)
                    + .045 * exp(-5*d) * sin(tau * 487 * d)
                    + .10 * exp(-32*d) * noise
                )
        yield (sample,)


def desk_scrape():
    rng = Random(452)
    noise, slow = 0., 0.
    for i in range(4 * RATE):
        t = i / RATE
        noise += .24 * (rng.uniform(-1, 1) - noise)
        slow += .055 * (noise - slow)
        d = t - .24
        if not 0 < d < 2.6:
            yield (0.,)
            continue
        envelope = sin(pi * d / 2.6) ** 1.7
        stutter = .34 + .66 * (.5 + .5 * sin(tau * (8 * d + 1.4 * sin(2 * d)))) ** 3
        rasp = .28 * (noise - .65 * slow) + .052 * sin(tau * (204 * d - 9 * d*d + .8 * sin(9 * d)))
        yield (envelope * stutter * rasp,)


if __name__ == '__main__':
    write_wave('SW_CorridorAir', corridor_air(), 2)
    write_wave('SW_Ballast', ballast(), 1)
    write_wave('SW_VentKnock', vent_knock(), 1)
    write_wave('SW_DistantDesk', desk_scrape(), 1)
