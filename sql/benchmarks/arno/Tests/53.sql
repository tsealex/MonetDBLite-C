SELECT MODEL237.is_mutagen, count(distinct MODEL237.model_id ) FROM MODEL MODEL237, BOND T1008290422850  WHERE MODEL237.model_id=T1008290422850.model_id AND MODEL237.logp='7' group by MODEL237.is_mutagen;
