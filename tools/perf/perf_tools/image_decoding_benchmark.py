# Copyright (c) 2012 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

from telemetry import multi_page_benchmark


class ImageDecoding(multi_page_benchmark.MultiPageBenchmark):
  def WillNavigateToPage(self, page, tab):
    tab.timeline.Start()

  def MeasurePage(self, page, tab, results):
    tab.timeline.Stop()
    def _IsDone():
      return tab.runtime.Evaluate('isDone')

    decode_image_events = \
        tab.timeline.timeline_events.GetAllOfType('DecodeImage')

    # If it is a real image benchmark, then store only the last-minIterations
    # decode tasks.
    if (hasattr(page,
                'image_decoding_benchmark_limit_results_to_min_iterations') and
        page.image_decoding_benchmark_limit_results_to_min_iterations):
      assert _IsDone()
      min_iterations = tab.runtime.Evaluate('minIterations')
      decode_image_events = decode_image_events[-min_iterations:]

    elapsed_times = [d.elapsed_time for d in decode_image_events]
    if not elapsed_times:
      results.Add('ImageDecoding_avg', 'ms', 'unsupported')
      return
    image_decoding_avg = sum(elapsed_times) / len(elapsed_times)
    results.Add('ImageDecoding_avg', 'ms', image_decoding_avg)
