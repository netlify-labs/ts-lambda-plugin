#pragma once

#include <ts/ts.h>
#include "plugin.h"

namespace tsLambda
{
    static struct {
        int average_latency = 0;
        int gauge_invocations = 0;
    } m_stats;

    static inline int initMetrics()
    {
        m_stats.average_latency = TSStatCreate(PLUGIN_NAME ".lambda.average_latency",
            TS_RECORDDATATYPE_INT, TS_STAT_NON_PERSISTENT, TS_STAT_SYNC_AVG);
        if (TS_ERROR == m_stats.average_latency)
        {
            return TS_ERROR;
        }

        TSStatIntSet(m_stats.average_latency, 0);

        m_stats.gauge_invocations = TSStatCreate(PLUGIN_NAME ".lambda.gauge_invocations",
            TS_RECORDDATATYPE_INT, TS_STAT_NON_PERSISTENT, TS_STAT_SYNC_SUM);
        if(TS_ERROR == m_stats.gauge_invocations)
        {
            return TS_ERROR;
        }
        TSStatIntSet(m_stats.gauge_invocations, 0);

        return TS_SUCCESS;
    };

    static inline void measureLatency(int latency) {
        TSStatIntIncrement(m_stats.average_latency, latency);
    };

    static inline void incrementInvocations() {
        TSStatIntIncrement(m_stats.gauge_invocations, 1);
    };
}