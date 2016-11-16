var req = require('request');
var ps = require('ps-node');

console.log('starting simulator.. looking for listener');


var url = 'http://127.0.0.1:4000';

var JSON = {
	"provider": {
		"name": "Dota 2",
		"appid": 570,
		"version": 44,
		"timestamp": 1478801650
	},
	"map": {
		"name": "hero_demo_main",
		"matchid": 0,
		"game_time": 2062,
		"clock_time": 2061,
		"daytime": true,
		"nightstalker_night": false,
		"game_state": "DOTA_GAMERULES_STATE_GAME_IN_PROGRESS",
		"win_team": "none",
		"customgamename": "hero_demo",
		"ward_purchase_cooldown": 0
	},
	"player": {
		"activity": "playing",
		"kills": 4,
		"deaths": 5,
		"assists": 0,
		"last_hits": 73,
		"denies": 0,
		"kill_streak": 4,
		"team_name": "radiant",
		"gold": 99999,
		"gold_reliable": 99999,
		"gold_unreliable": 0,
		"gpm": 8277,
		"xpm": 830
	},
	"hero": {
		"id": 94,
		"name": "npc_dota_hero_medusa",
		"level": "23",
		"alive": true,
		"respawn_seconds": 0,
		"buyback_cost": 1408,
		"buyback_cooldown": 0,
		"health": 2265,
		"max_health": 2265,
		"health_percent": 100,
		"mana": 2134,
		"max_mana": 2134,
		"mana_percent": 100,
		"silenced": false,
		"stunned": false,
		"disarmed": false,
		"magicimmune": false,
		"hexed": false,
		"muted": false,
		"break": false,
		"has_debuff": false
	}
}; 

var sendJSON = function() {
    req({
        uri: url,
        method: "POST",
        json: true,
        body: JSON
    }, function (error, response, body) {
        if(error) {
            console.log(error);
        }
    });
    JSON.hero.health_percent+=5;
    JSON.hero.health_percent %= 100;
    JSON.hero.mana_percent+=5;
    JSON.hero.mana_percent %= 100;
}

ps.lookup({
    command: 'node',
    arguments: 'gsi-listener',
}, function(err, resultList) {
    if(resultList.length == 0) {
        console.log('no listener found, please run the gsi-listener');
        process.exit();
    } else {
        console.log('listener found, simulating data now..');
        setInterval(sendJSON, 2000);
    }
});