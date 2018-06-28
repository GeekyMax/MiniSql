//
//  Attribute.cpp
//  MiniSQL
//
//  Created by HoraceCai on 2018/6/23.
//  Copyright © 2018年 apple. All rights reserved.
//

#include "Attribute.h"

Attribute::Attribute(string n, int t, bool i) {
    name = n;
    type = t;
    ifUnique = i;
    index = "";
    
}
