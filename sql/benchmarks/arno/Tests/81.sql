SELECT MODEL286.is_mutagen,MODEL286.logp, count(distinct MODEL286.model_id ) FROM MODEL MODEL286  WHERE MODEL286.logp='2' group by MODEL286.logp , MODEL286.is_mutagen;
