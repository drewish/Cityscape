//
//  ZoningPlanner.h
//  Cityscape
//
//  Created by Andrew Morton on 1/9/17.
//
//

#pragma once

namespace Cityscape {

struct ZoningPlan;
typedef std::shared_ptr<ZoningPlan>     ZoningPlanRef;

ZoningPlanRef zoneMajesticHeights();
ZoningPlanRef zoneFarming();
ZoningPlanRef zoneIndustrial();
ZoningPlanRef zoneDowntown();

} // namespace Cityscape
