function on_load()
    print("[hello_cyan] on_load")
end

function on_update(dt)
end

function on_draw()
    draw.rect(10, 10, 60, 30, 200, 50, 50)
    draw.text(15, 45, "hello!", 16, 255, 255, 255)
end

function on_event(type)
    if type == Event.BUTTON1_DOWN then
        print("[hello_cyan] button 1 pressed")
    end
    if type == Event.BUTTON1_UP then
        print("[hello_cyan] button 1 up")
    end
end

function on_unload()
    print("[hello_cyan] on_unload")
end