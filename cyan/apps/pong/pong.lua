local paddleYp1 = 0
local paddleYp2 = 0

local ballX = 0
local ballY = 0
local ballXvel = 0
local ballYvel = 0
local ballThickness = 10
local ballSpeed = 300

local paddleWidth = 6
local paddleHeight = 0
local paddleMargin = 10
local paddleSpeed = 10
local aiSpeed = 110

local p1score = 0
local p2score = 0

local function clampPaddle(y)
    local minY = paddleMargin
    local maxY = draw.height() - paddleMargin - paddleHeight
    if y < minY then return minY end
    if y > maxY then return maxY end
    return y
end

local function resetLayout()
    paddleHeight = draw.height() * 0.3
    paddleYp1 = clampPaddle((draw.height() - paddleHeight) / 2)
    paddleYp2 = clampPaddle((draw.height() - paddleHeight) / 2)
    ballX = draw.width() / 2
    ballY = draw.height() / 2
end

local function resetBall(direction)
    ballX = draw.width() / 2
    ballY = draw.height() / 2
    ballXvel = ballSpeed * direction
    ballYvel = (p1score + p2score) % 2 == 0 and ballSpeed * 0.5 or -ballSpeed * 0.5
end

function on_load()
    print("on_load")
    resetLayout()
    resetBall(1)
end

function on_update(dt)
    local w = draw.width()
    local h = draw.height()
    local wallThickness = 2
    local halfBall = ballThickness / 2

    local paddle2Center = paddleYp2 + paddleHeight / 2
    local diff = ballY - paddle2Center
    local maxMove = aiSpeed * dt
    if diff > maxMove then diff = maxMove
    elseif diff < -maxMove then diff = -maxMove end
    paddleYp2 = clampPaddle(paddleYp2 + diff)

    ballX = ballX + ballXvel * dt
    ballY = ballY + ballYvel * dt

    if ballY - halfBall <= wallThickness then
        ballY = wallThickness + halfBall
        ballYvel = -ballYvel
    elseif ballY + halfBall >= h - wallThickness then
        ballY = h - wallThickness - halfBall
        ballYvel = -ballYvel
    end

    if ballXvel < 0
        and ballX - halfBall <= paddleMargin + paddleWidth
        and ballX - halfBall >= paddleMargin
        and ballY + halfBall >= paddleYp1
        and ballY - halfBall <= paddleYp1 + paddleHeight then
        ballX = paddleMargin + paddleWidth + halfBall
        ballXvel = -ballXvel
    end

    local rightPaddleX = w - paddleMargin - paddleWidth
    if ballXvel > 0
        and ballX + halfBall >= rightPaddleX
        and ballX + halfBall <= rightPaddleX + paddleWidth
        and ballY + halfBall >= paddleYp2
        and ballY - halfBall <= paddleYp2 + paddleHeight then
        ballX = rightPaddleX - halfBall
        ballXvel = -ballXvel
    end

    if ballX + halfBall < 0 then
        p2score = p2score + 1
        print("p2 scores: " .. p1score .. " - " .. p2score)
        resetBall(-1)
    elseif ballX - halfBall > w then
        p1score = p1score + 1
        print("p1 scores: " .. p1score .. " - " .. p2score)
        resetBall(1)
    end
end

function on_event(type)
    if type == Event.DISPLAY_ALTERED then
        print("display altered")
        resetLayout()
    elseif type == Event.SCROLL_UP then
        paddleYp1 = clampPaddle(paddleYp1 - paddleSpeed)
    elseif type == Event.SCROLL_DOWN then
        paddleYp1 = clampPaddle(paddleYp1 + paddleSpeed)
    end
end

function on_draw()
    local w = draw.width()
    local h = draw.height()
    local thickness = 2

    draw.rect(0, 0, w, thickness, 255, 255, 255)
    draw.rect(0, h - thickness, w, thickness, 255, 255, 255)
    draw.rect(0, 0, thickness, h, 255, 255, 255)
    draw.rect(w - thickness, 0, thickness, h, 255, 255, 255)

    local dashHeight = 8
    local gap = 6
    local x = (w // 2) - (thickness // 2)
    local y = 0
    while y < h do
        draw.rect(x, y, thickness, dashHeight, 255, 255, 255)
        y = y + dashHeight + gap
    end

    -- player 1
    draw.rect(paddleMargin, paddleYp1, paddleWidth, paddleHeight, 200, 200, 200)
    -- player 2 (AI)
    draw.rect(w - paddleMargin - paddleWidth, paddleYp2, paddleWidth, paddleHeight, 200, 200, 200)
    -- ball
    draw.rect(ballX - (ballThickness // 2), ballY - (ballThickness // 2), ballThickness, ballThickness, 200, 200, 200)

    draw.text((w // 4), 10, tostring(p1score), 16, 255, 255, 255)
    draw.text((w // 4) * 3, 10, tostring(p2score), 16, 255, 255, 255)
end

function on_unload()
    print("on_unload")
end