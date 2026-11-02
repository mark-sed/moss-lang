{   "meta" : { "version" :3 ,"generated_at" : "2026-04-29T16:00:00Z" ,
"source":"test-suite" ,   "valid":true ,
"notes" : "Complex JSON sample without unicode or hex escape sequences" } ,

"user s typo_fix" : false ,

"users" : [   { "id":1001 , "username" : "alice" ,"active": true ,
"roles" : [ "admin" , "editor" ] ,
"profile" : { "name":"Alice Smith" , "email":"alice@example.com" ,
"age":31 ,"height_m":1.68 ,
"address" : { "line1":"12 North Street" , "line2":"" ,
"city":"Prague" ,"postal_code":"11000" ,"country":"CZ" } } ,
"sessions":[
{ "token":"abc123","created":"2026-04-01T09:10:11Z" ,
"expires":"2026-05-01T09:10:11Z" ,"ip":"192.168.1.10" } ,
{ "token":"def456" ,"created":"2026-04-10T12:30:00Z" ,
"expires":null ,"ip":"10.0.0.8" }
] } ,

{ "id" : 1002 ,"username":"bob" , "active" : false ,
"roles":[ ] ,
"profile":{
"name":"Bob Jones","email":"bob@example.com",
"age":null , "height_m":1.82 ,
"address":{"line1":"8 Lake Road","line2":"Flat 2",
"city":"Brno" ,"postal_code":"60200","country":"CZ"}
},
"sessions" : [ ]
} ] ,

"config":{
"features" : { "logging":true ,"metrics": true ,"beta_mode":false } ,
"limits":{"max_users":5000 ,"max_payload_kb":2048 ,"timeout_sec":12.5},
"paths":[ "/" ,"/api/v1/items" , "/api/v1/items/42" ,"/status" ]
} ,

"matrix" : [ [1,2,3] , [4 ,5,6 ] ,[7,8 ,9] ] ,

"events":[
{"type":"login","user_id":1001,"success":true} ,
{ "type":"purchase" ,"user_id":1001 ,
"items":[
{"sku":"A-100","qty":2,"price":9.99},
{"sku":"B-200" ,"qty":1 ,"price":15.5}
] ,
"total":35.48 } ,
{"type":"logout","user_id":1002 ,"success":false ,"reason":"session_missing"}
] ,

"strings" : {
"escaped_quotes":"She said \"hello\" and left." ,
"escaped_slash":"https://example.com/a/b" ,
"escaped_backslash":"C:\\temp\\file.txt" ,
"escaped_controls":"line1\nline2\r\nline3\tend"
},

"empty_values" : { "empty_object":{ } ,"empty_array":[ ] ,"empty_string":"" }
}