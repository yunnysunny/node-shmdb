{
    "targets" : [
        {
            "target_name" : "shmdb",
            "include_dirs" : [
				"<!(node -e \"require('nan')\")",
                "include",
				"./extenal/shmdb/include"
            ],
            
            
            "sources" : [
                "addon.cc",
                "ShmdbObject.cc",
				"extenal/shmdb/src/hash.c",
				"extenal/shmdb/src/log.c",
				"extenal/shmdb/src/mm.c",
				"extenal/shmdb/src/prime.c",
				"extenal/shmdb/src/transform.c"
            ]
        }
    ]
}
