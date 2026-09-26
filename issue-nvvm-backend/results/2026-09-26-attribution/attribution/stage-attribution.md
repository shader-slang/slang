# Material compilation stage attribution

Partition every sample before computing inclusive quartiles and medians; warmups excluded. Both rounds retained. Displayed phase resolution is 0.01ms; residuals within 0.08ms of zero are rounded to zero. Medians and percentages need not add up across stages.

NVVM target work includes legalization, planning, provider loading, capability checks, construction, serialization and module teardown; NVRTC target work is source emission excluding linking. Vendor compile is an opaque API duration, not pure optimization. Zero separate NVRTC verification means no separate exposed call, not absence of internal validation. No assembly or GPU time is included.

| Cell / round | Stage | Median ms (IQR) | Median % wall (IQR) |
| --- | --- | ---: | ---: |
| tiled-brass-material-eval_buffer-nvrtc-o3 / combined | Built-in module loading | 217.13 (211.41–219.89) | 15.58 (15.28–15.87) |
| tiled-brass-material-eval_buffer-nvrtc-o3 / combined | Slang front end (includes semantic checking and IR generation) | 552.93 (546.54–562.24) | 39.70 (39.35–40.03) |
| tiled-brass-material-eval_buffer-nvrtc-o3 / combined | Slang IR linking and optimization | 307.42 (301.84–318.85) | 22.09 (21.88–22.74) |
| tiled-brass-material-eval_buffer-nvrtc-o3 / combined | Target preparation/emission, including NVVM serialization | 7.68 (7.60–7.77) | 0.55 (0.54–0.56) |
| tiled-brass-material-eval_buffer-nvrtc-o3 / combined | Separate libNVVM verification call (none exposed for NVRTC) | 0.00 (0.00–0.00) | 0.00 (0.00–0.00) |
| tiled-brass-material-eval_buffer-nvrtc-o3 / combined | Vendor compile API call, including NVRTC source compilation | 200.65 (195.37–206.40) | 14.35 (14.16–14.60) |
| tiled-brass-material-eval_buffer-nvrtc-o3 / combined | Remaining fresh-process wall time, including startup/teardown and library loading | 105.28 (103.37–106.38) | 7.48 (7.30–7.71) |
| tiled-brass-material-eval_buffer-nvrtc-o3 / 0 | Built-in module loading | 220.19 (217.24–228.90) | 15.73 (15.23–15.88) |
| tiled-brass-material-eval_buffer-nvrtc-o3 / 0 | Slang front end (includes semantic checking and IR generation) | 563.46 (556.37–573.25) | 39.68 (39.41–39.90) |
| tiled-brass-material-eval_buffer-nvrtc-o3 / 0 | Slang IR linking and optimization | 317.20 (306.70–334.54) | 22.41 (21.44–23.26) |
| tiled-brass-material-eval_buffer-nvrtc-o3 / 0 | Target preparation/emission, including NVVM serialization | 7.75 (7.67–7.77) | 0.54 (0.53–0.55) |
| tiled-brass-material-eval_buffer-nvrtc-o3 / 0 | Separate libNVVM verification call (none exposed for NVRTC) | 0.00 (0.00–0.00) | 0.00 (0.00–0.00) |
| tiled-brass-material-eval_buffer-nvrtc-o3 / 0 | Vendor compile API call, including NVRTC source compilation | 203.58 (200.08–211.06) | 14.37 (14.15–14.76) |
| tiled-brass-material-eval_buffer-nvrtc-o3 / 0 | Remaining fresh-process wall time, including startup/teardown and library loading | 105.14 (103.41–105.83) | 7.30 (7.20–7.50) |
| tiled-brass-material-eval_buffer-nvrtc-o3 / 1 | Built-in module loading | 215.49 (211.03–217.03) | 15.47 (15.31–15.82) |
| tiled-brass-material-eval_buffer-nvrtc-o3 / 1 | Slang front end (includes semantic checking and IR generation) | 548.18 (539.88–551.64) | 39.73 (39.34–40.06) |
| tiled-brass-material-eval_buffer-nvrtc-o3 / 1 | Slang IR linking and optimization | 303.28 (300.07–308.15) | 22.09 (21.95–22.19) |
| tiled-brass-material-eval_buffer-nvrtc-o3 / 1 | Target preparation/emission, including NVVM serialization | 7.64 (7.60–7.69) | 0.56 (0.55–0.56) |
| tiled-brass-material-eval_buffer-nvrtc-o3 / 1 | Separate libNVVM verification call (none exposed for NVRTC) | 0.00 (0.00–0.00) | 0.00 (0.00–0.00) |
| tiled-brass-material-eval_buffer-nvrtc-o3 / 1 | Vendor compile API call, including NVRTC source compilation | 197.12 (194.74–201.22) | 14.34 (14.18–14.49) |
| tiled-brass-material-eval_buffer-nvrtc-o3 / 1 | Remaining fresh-process wall time, including startup/teardown and library loading | 105.42 (103.36–106.76) | 7.70 (7.45–7.76) |
| tiled-brass-material-eval_buffer-nvvm-o0 / combined | Built-in module loading | 212.66 (207.81–221.27) | 16.43 (16.29–16.59) |
| tiled-brass-material-eval_buffer-nvvm-o0 / combined | Slang front end (includes semantic checking and IR generation) | 551.81 (532.14–573.37) | 42.25 (41.78–42.68) |
| tiled-brass-material-eval_buffer-nvvm-o0 / combined | Slang IR linking and optimization | 307.56 (298.90–316.88) | 23.52 (23.22–23.98) |
| tiled-brass-material-eval_buffer-nvvm-o0 / combined | Target preparation/emission, including NVVM serialization | 36.78 (36.53–38.57) | 2.89 (2.84–2.93) |
| tiled-brass-material-eval_buffer-nvvm-o0 / combined | Separate libNVVM verification call (none exposed for NVRTC) | 10.41 (10.37–10.58) | 0.81 (0.78–0.84) |
| tiled-brass-material-eval_buffer-nvvm-o0 / combined | Vendor compile API call, including NVRTC source compilation | 79.47 (78.95–82.02) | 6.17 (6.12–6.22) |
| tiled-brass-material-eval_buffer-nvvm-o0 / combined | Remaining fresh-process wall time, including startup/teardown and library loading | 102.37 (98.96–104.93) | 7.70 (7.58–8.08) |
| tiled-brass-material-eval_buffer-nvvm-o0 / 0 | Built-in module loading | 221.38 (214.75–222.66) | 16.43 (16.31–16.61) |
| tiled-brass-material-eval_buffer-nvvm-o0 / 0 | Slang front end (includes semantic checking and IR generation) | 573.95 (568.55–576.17) | 42.74 (42.13–43.11) |
| tiled-brass-material-eval_buffer-nvvm-o0 / 0 | Slang IR linking and optimization | 312.18 (309.51–323.49) | 23.51 (22.95–24.09) |
| tiled-brass-material-eval_buffer-nvvm-o0 / 0 | Target preparation/emission, including NVVM serialization | 38.82 (36.73–41.17) | 2.88 (2.75–3.07) |
| tiled-brass-material-eval_buffer-nvvm-o0 / 0 | Separate libNVVM verification call (none exposed for NVRTC) | 10.55 (10.40–13.39) | 0.78 (0.77–0.99) |
| tiled-brass-material-eval_buffer-nvvm-o0 / 0 | Vendor compile API call, including NVRTC source compilation | 82.36 (79.85–83.22) | 6.17 (6.00–6.17) |
| tiled-brass-material-eval_buffer-nvvm-o0 / 0 | Remaining fresh-process wall time, including startup/teardown and library loading | 102.31 (100.25–104.70) | 7.61 (7.50–7.69) |
| tiled-brass-material-eval_buffer-nvvm-o0 / 1 | Built-in module loading | 207.75 (207.70–210.52) | 16.43 (16.29–16.53) |
| tiled-brass-material-eval_buffer-nvvm-o0 / 1 | Slang front end (includes semantic checking and IR generation) | 531.87 (528.76–534.44) | 41.96 (41.71–42.38) |
| tiled-brass-material-eval_buffer-nvvm-o0 / 1 | Slang IR linking and optimization | 298.82 (295.17–301.07) | 23.53 (23.50–23.60) |
| tiled-brass-material-eval_buffer-nvvm-o0 / 1 | Target preparation/emission, including NVVM serialization | 36.63 (36.49–36.83) | 2.91 (2.86–2.91) |
| tiled-brass-material-eval_buffer-nvvm-o0 / 1 | Separate libNVVM verification call (none exposed for NVRTC) | 10.39 (10.34–10.43) | 0.82 (0.81–0.83) |
| tiled-brass-material-eval_buffer-nvvm-o0 / 1 | Vendor compile API call, including NVRTC source compilation | 78.94 (78.59–79.10) | 6.22 (6.17–6.26) |
| tiled-brass-material-eval_buffer-nvvm-o0 / 1 | Remaining fresh-process wall time, including startup/teardown and library loading | 102.43 (98.70–105.01) | 8.10 (7.77–8.28) |
| tiled-brass-material-eval_buffer-nvvm-o3 / combined | Built-in module loading | 215.67 (212.35–219.27) | 15.39 (15.17–15.55) |
| tiled-brass-material-eval_buffer-nvvm-o3 / combined | Slang front end (includes semantic checking and IR generation) | 549.98 (539.57–559.62) | 39.11 (38.98–39.47) |
| tiled-brass-material-eval_buffer-nvvm-o3 / combined | Slang IR linking and optimization | 306.15 (303.59–310.58) | 21.83 (21.66–22.00) |
| tiled-brass-material-eval_buffer-nvvm-o3 / combined | Target preparation/emission, including NVVM serialization | 36.62 (36.38–39.24) | 2.62 (2.60–2.74) |
| tiled-brass-material-eval_buffer-nvvm-o3 / combined | Separate libNVVM verification call (none exposed for NVRTC) | 10.43 (10.35–10.55) | 0.75 (0.73–0.76) |
| tiled-brass-material-eval_buffer-nvvm-o3 / combined | Vendor compile API call, including NVRTC source compilation | 185.44 (182.39–188.61) | 13.28 (13.11–13.35) |
| tiled-brass-material-eval_buffer-nvvm-o3 / combined | Remaining fresh-process wall time, including startup/teardown and library loading | 96.71 (95.23–99.60) | 6.92 (6.74–7.16) |
| tiled-brass-material-eval_buffer-nvvm-o3 / 0 | Built-in module loading | 215.25 (213.04–219.76) | 15.23 (15.15–15.42) |
| tiled-brass-material-eval_buffer-nvvm-o3 / 0 | Slang front end (includes semantic checking and IR generation) | 558.77 (548.83–570.09) | 39.19 (39.10–39.50) |
| tiled-brass-material-eval_buffer-nvvm-o3 / 0 | Slang IR linking and optimization | 306.23 (304.70–311.47) | 21.83 (21.69–21.87) |
| tiled-brass-material-eval_buffer-nvvm-o3 / 0 | Target preparation/emission, including NVVM serialization | 36.68 (36.57–40.21) | 2.61 (2.57–2.80) |
| tiled-brass-material-eval_buffer-nvvm-o3 / 0 | Separate libNVVM verification call (none exposed for NVRTC) | 10.54 (10.43–10.75) | 0.74 (0.73–0.75) |
| tiled-brass-material-eval_buffer-nvvm-o3 / 0 | Vendor compile API call, including NVRTC source compilation | 187.45 (185.74–191.77) | 13.31 (13.10–13.37) |
| tiled-brass-material-eval_buffer-nvvm-o3 / 0 | Remaining fresh-process wall time, including startup/teardown and library loading | 99.84 (96.63–101.83) | 7.08 (6.73–7.23) |
| tiled-brass-material-eval_buffer-nvvm-o3 / 1 | Built-in module loading | 216.08 (209.47–217.82) | 15.52 (15.31–15.71) |
| tiled-brass-material-eval_buffer-nvvm-o3 / 1 | Slang front end (includes semantic checking and IR generation) | 538.37 (533.70–550.49) | 39.07 (38.71–39.34) |
| tiled-brass-material-eval_buffer-nvvm-o3 / 1 | Slang IR linking and optimization | 305.73 (300.18–306.82) | 21.83 (21.65–22.12) |
| tiled-brass-material-eval_buffer-nvvm-o3 / 1 | Target preparation/emission, including NVVM serialization | 36.45 (36.29–36.66) | 2.67 (2.61–2.68) |
| tiled-brass-material-eval_buffer-nvvm-o3 / 1 | Separate libNVVM verification call (none exposed for NVRTC) | 10.38 (10.33–10.44) | 0.75 (0.73–0.76) |
| tiled-brass-material-eval_buffer-nvvm-o3 / 1 | Vendor compile API call, including NVRTC source compilation | 183.09 (181.05–185.14) | 13.25 (13.12–13.32) |
| tiled-brass-material-eval_buffer-nvvm-o3 / 1 | Remaining fresh-process wall time, including startup/teardown and library loading | 96.12 (95.23–96.80) | 6.87 (6.74–6.95) |
| tiled-brass-material-sample_buffer-nvrtc-o3 / combined | Built-in module loading | 217.59 (215.37–219.74) | 15.22 (15.10–15.40) |
| tiled-brass-material-sample_buffer-nvrtc-o3 / combined | Slang front end (includes semantic checking and IR generation) | 557.71 (544.47–569.77) | 38.86 (38.62–39.53) |
| tiled-brass-material-sample_buffer-nvrtc-o3 / combined | Slang IR linking and optimization | 325.97 (321.15–339.81) | 22.87 (22.58–23.27) |
| tiled-brass-material-sample_buffer-nvrtc-o3 / combined | Target preparation/emission, including NVVM serialization | 9.03 (8.83–9.12) | 0.63 (0.62–0.64) |
| tiled-brass-material-sample_buffer-nvrtc-o3 / combined | Separate libNVVM verification call (none exposed for NVRTC) | 0.00 (0.00–0.00) | 0.00 (0.00–0.00) |
| tiled-brass-material-sample_buffer-nvrtc-o3 / combined | Vendor compile API call, including NVRTC source compilation | 207.07 (204.42–211.33) | 14.57 (14.35–14.77) |
| tiled-brass-material-sample_buffer-nvrtc-o3 / combined | Remaining fresh-process wall time, including startup/teardown and library loading | 106.10 (104.65–107.48) | 7.40 (7.32–7.62) |
| tiled-brass-material-sample_buffer-nvrtc-o3 / 0 | Built-in module loading | 219.78 (218.85–222.50) | 15.19 (15.10–15.30) |
| tiled-brass-material-sample_buffer-nvrtc-o3 / 0 | Slang front end (includes semantic checking and IR generation) | 570.08 (560.70–603.51) | 38.92 (38.71–39.63) |
| tiled-brass-material-sample_buffer-nvrtc-o3 / 0 | Slang IR linking and optimization | 335.83 (324.63–357.08) | 22.73 (22.39–23.05) |
| tiled-brass-material-sample_buffer-nvrtc-o3 / 0 | Target preparation/emission, including NVVM serialization | 9.07 (9.04–9.21) | 0.62 (0.59–0.63) |
| tiled-brass-material-sample_buffer-nvrtc-o3 / 0 | Separate libNVVM verification call (none exposed for NVRTC) | 0.00 (0.00–0.00) | 0.00 (0.00–0.00) |
| tiled-brass-material-sample_buffer-nvrtc-o3 / 0 | Vendor compile API call, including NVRTC source compilation | 211.88 (208.38–224.30) | 14.56 (14.37–14.60) |
| tiled-brass-material-sample_buffer-nvrtc-o3 / 0 | Remaining fresh-process wall time, including startup/teardown and library loading | 106.72 (106.37–119.46) | 7.39 (7.32–7.66) |
| tiled-brass-material-sample_buffer-nvrtc-o3 / 1 | Built-in module loading | 215.36 (212.92–216.07) | 15.24 (15.16–15.41) |
| tiled-brass-material-sample_buffer-nvrtc-o3 / 1 | Slang front end (includes semantic checking and IR generation) | 544.17 (542.65–548.96) | 38.69 (38.38–39.44) |
| tiled-brass-material-sample_buffer-nvrtc-o3 / 1 | Slang IR linking and optimization | 321.76 (319.25–330.06) | 23.05 (22.66–23.55) |
| tiled-brass-material-sample_buffer-nvrtc-o3 / 1 | Target preparation/emission, including NVVM serialization | 8.82 (8.80–8.97) | 0.63 (0.63–0.64) |
| tiled-brass-material-sample_buffer-nvrtc-o3 / 1 | Separate libNVVM verification call (none exposed for NVRTC) | 0.00 (0.00–0.00) | 0.00 (0.00–0.00) |
| tiled-brass-material-sample_buffer-nvrtc-o3 / 1 | Vendor compile API call, including NVRTC source compilation | 204.16 (204.05–206.84) | 14.61 (14.34–14.80) |
| tiled-brass-material-sample_buffer-nvrtc-o3 / 1 | Remaining fresh-process wall time, including startup/teardown and library loading | 105.51 (102.49–105.84) | 7.40 (7.33–7.55) |
| tiled-brass-material-sample_buffer-nvvm-o0 / combined | Built-in module loading | 217.62 (210.36–248.25) | 16.03 (15.52–16.33) |
| tiled-brass-material-sample_buffer-nvvm-o0 / combined | Slang front end (includes semantic checking and IR generation) | 567.97 (528.73–662.30) | 40.71 (40.22–41.02) |
| tiled-brass-material-sample_buffer-nvvm-o0 / combined | Slang IR linking and optimization | 330.78 (313.83–396.46) | 24.05 (23.79–25.07) |
| tiled-brass-material-sample_buffer-nvvm-o0 / combined | Target preparation/emission, including NVVM serialization | 43.17 (42.58–48.19) | 3.25 (2.89–3.30) |
| tiled-brass-material-sample_buffer-nvvm-o0 / combined | Separate libNVVM verification call (none exposed for NVRTC) | 12.00 (11.72–14.88) | 0.90 (0.89–0.93) |
| tiled-brass-material-sample_buffer-nvvm-o0 / combined | Vendor compile API call, including NVRTC source compilation | 96.91 (93.32–104.71) | 7.08 (6.59–7.20) |
| tiled-brass-material-sample_buffer-nvvm-o0 / combined | Remaining fresh-process wall time, including startup/teardown and library loading | 107.05 (99.61–111.20) | 7.31 (6.97–7.69) |
| tiled-brass-material-sample_buffer-nvvm-o0 / 0 | Built-in module loading | 251.55 (237.01–256.33) | 15.46 (14.80–16.27) |
| tiled-brass-material-sample_buffer-nvvm-o0 / 0 | Slang front end (includes semantic checking and IR generation) | 669.65 (621.71–700.65) | 40.61 (40.20–43.75) |
| tiled-brass-material-sample_buffer-nvvm-o0 / 0 | Slang IR linking and optimization | 397.49 (361.22–441.74) | 24.75 (22.67–27.15) |
| tiled-brass-material-sample_buffer-nvvm-o0 / 0 | Target preparation/emission, including NVVM serialization | 49.42 (43.99–50.71) | 2.88 (2.77–3.04) |
| tiled-brass-material-sample_buffer-nvvm-o0 / 0 | Separate libNVVM verification call (none exposed for NVRTC) | 15.25 (12.14–15.75) | 0.94 (0.79–1.01) |
| tiled-brass-material-sample_buffer-nvvm-o0 / 0 | Vendor compile API call, including NVRTC source compilation | 104.75 (103.86–115.02) | 6.57 (6.50–7.03) |
| tiled-brass-material-sample_buffer-nvvm-o0 / 0 | Remaining fresh-process wall time, including startup/teardown and library loading | 109.87 (109.03–118.65) | 6.98 (6.86–7.45) |
| tiled-brass-material-sample_buffer-nvvm-o0 / 1 | Built-in module loading | 210.10 (208.61–211.54) | 16.06 (15.98–16.34) |
| tiled-brass-material-sample_buffer-nvvm-o0 / 1 | Slang front end (includes semantic checking and IR generation) | 528.17 (524.19–534.52) | 40.77 (40.56–40.82) |
| tiled-brass-material-sample_buffer-nvvm-o0 / 1 | Slang IR linking and optimization | 313.37 (309.85–316.64) | 24.04 (23.94–24.17) |
| tiled-brass-material-sample_buffer-nvvm-o0 / 1 | Target preparation/emission, including NVVM serialization | 42.56 (42.38–43.01) | 3.27 (3.25–3.30) |
| tiled-brass-material-sample_buffer-nvvm-o0 / 1 | Separate libNVVM verification call (none exposed for NVRTC) | 11.71 (11.61–11.78) | 0.90 (0.89–0.90) |
| tiled-brass-material-sample_buffer-nvvm-o0 / 1 | Vendor compile API call, including NVRTC source compilation | 93.32 (92.75–93.63) | 7.20 (7.10–7.22) |
| tiled-brass-material-sample_buffer-nvvm-o0 / 1 | Remaining fresh-process wall time, including startup/teardown and library loading | 99.33 (92.78–101.87) | 7.63 (7.18–7.86) |
| tiled-brass-material-sample_buffer-nvvm-o3 / combined | Built-in module loading | 220.04 (214.30–240.44) | 14.19 (13.79–14.65) |
| tiled-brass-material-sample_buffer-nvvm-o3 / combined | Slang front end (includes semantic checking and IR generation) | 579.89 (538.65–642.24) | 36.71 (36.29–37.57) |
| tiled-brass-material-sample_buffer-nvvm-o3 / combined | Slang IR linking and optimization | 332.43 (315.13–380.44) | 21.40 (21.03–21.81) |
| tiled-brass-material-sample_buffer-nvvm-o3 / combined | Target preparation/emission, including NVVM serialization | 45.41 (42.54–53.95) | 2.95 (2.88–3.17) |
| tiled-brass-material-sample_buffer-nvvm-o3 / combined | Separate libNVVM verification call (none exposed for NVRTC) | 11.88 (11.58–14.02) | 0.79 (0.77–0.80) |
| tiled-brass-material-sample_buffer-nvvm-o3 / combined | Vendor compile API call, including NVRTC source compilation | 283.50 (259.99–293.34) | 17.65 (17.02–17.93) |
| tiled-brass-material-sample_buffer-nvvm-o3 / combined | Remaining fresh-process wall time, including startup/teardown and library loading | 97.31 (94.91–100.93) | 6.22 (5.85–6.42) |
| tiled-brass-material-sample_buffer-nvvm-o3 / 0 | Built-in module loading | 230.61 (216.90–247.73) | 14.25 (14.05–14.71) |
| tiled-brass-material-sample_buffer-nvvm-o3 / 0 | Slang front end (includes semantic checking and IR generation) | 603.71 (576.27–633.44) | 37.22 (36.57–37.72) |
| tiled-brass-material-sample_buffer-nvvm-o3 / 0 | Slang IR linking and optimization | 337.77 (316.56–349.79) | 21.25 (20.72–21.47) |
| tiled-brass-material-sample_buffer-nvvm-o3 / 0 | Target preparation/emission, including NVVM serialization | 42.96 (42.54–52.48) | 2.88 (2.78–3.00) |
| tiled-brass-material-sample_buffer-nvvm-o3 / 0 | Separate libNVVM verification call (none exposed for NVRTC) | 11.94 (11.53–15.22) | 0.77 (0.74–0.95) |
| tiled-brass-material-sample_buffer-nvvm-o3 / 0 | Vendor compile API call, including NVRTC source compilation | 283.30 (260.72–287.16) | 17.11 (16.96–17.70) |
| tiled-brass-material-sample_buffer-nvvm-o3 / 0 | Remaining fresh-process wall time, including startup/teardown and library loading | 98.61 (94.97–100.40) | 6.01 (5.83–6.37) |
| tiled-brass-material-sample_buffer-nvvm-o3 / 1 | Built-in module loading | 216.92 (213.71–236.00) | 14.01 (13.48–14.26) |
| tiled-brass-material-sample_buffer-nvvm-o3 / 1 | Slang front end (includes semantic checking and IR generation) | 538.64 (533.77–645.17) | 36.48 (35.85–36.86) |
| tiled-brass-material-sample_buffer-nvvm-o3 / 1 | Slang IR linking and optimization | 325.90 (314.66–384.19) | 21.45 (21.08–22.02) |
| tiled-brass-material-sample_buffer-nvvm-o3 / 1 | Target preparation/emission, including NVVM serialization | 52.30 (42.55–56.64) | 2.99 (2.90–3.26) |
| tiled-brass-material-sample_buffer-nvvm-o3 / 1 | Separate libNVVM verification call (none exposed for NVRTC) | 11.81 (11.65–14.00) | 0.79 (0.78–0.80) |
| tiled-brass-material-sample_buffer-nvvm-o3 / 1 | Vendor compile API call, including NVRTC source compilation | 283.71 (259.75–295.35) | 17.89 (17.60–17.95) |
| tiled-brass-material-sample_buffer-nvvm-o3 / 1 | Remaining fresh-process wall time, including startup/teardown and library loading | 96.02 (94.88–100.96) | 6.41 (5.89–6.53) |

![Independent stage medians](stage-attribution.svg)

Full statistics and provenance: [stage-attribution.json](stage-attribution.json).
