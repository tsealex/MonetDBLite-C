(: q5 'SELECT alb.Title, ass.Title FROM Assets as ass, Albums as alb WHERE ass.AlbumId=alb.AlbumId and ass.TrackNr=1' :)
import module namespace music = "http://www.cwi.nl/~boncz/music/opt/" at "http://www.cwi.nl/~boncz/music/opt/music.mil";
music:AlbumAssetByTrackNr1("music.xml", 1000000000)
