from collections import namedtuple

Op = namedtuple('Op', ('name', 'format', 'is_audio_thread', 'desc_short', 'desc_long'))

ops = [
    Op('voice buffer', 'ii', False, 'set audio buffer index for given voice', ''),

    Op('voice rate', 'if', True, 'set head movement speed for given voice', ''),
    Op('voice position', 'if', True, 'queue a position change in seconds for given voice', ''),
    Op('voice loop start', 'if', True, 'set loop start point in seconds for given voice', ''),
    Op('voice loop end', 'if', True, 'set loop end point in seconds for given voice', ''),
    Op('voice fade time', 'if', True, 'crossfade time in seconds for given voice', ''),

    Op('voice loop flag', 'ib', True, 'enable/disable looping for given voice', ''),
    Op('voice read flag', 'ib', True, 'enable/disable reading (playback) for given voice', ''),
    Op('voice write flag', 'ib', True, 'enable/disable writing (record) for given voice', ''),

    Op('voice pre filter enabled', 'if', True, '', ''),
    Op('voice pre filter fc mod', 'if', True, '', ''),
    Op('voice pre filter q', 'if', True, '', ''),
    
    Op('voice post filter fc', 'if', True, '', ''),
    Op('voice post filter rq',  'if', True, '', ''),
    Op('voice post filter lp', 'if', True, '', ''),
    Op('voice post filter hp', 'if', True, '', ''),
    Op('voice post filter bp', 'if', True, '', ''),
    Op('voice post filter br', 'if', True, '', ''),
    Op('voice post filter dry', 'if', True, '', ''),
    
    Op('voice rec pre slew time', 'if', True, '', ''),
    Op('voice rate slew time', 'if', True, '', ''),
    
    Op('voice read duck', 'ii', True, 'set read-ducking target', ''),
    Op('voice write duck', 'ii', True, 'set write-ducking target', ''),
    
    Op('voice phase quant', 'if', True, '', ''),
]
