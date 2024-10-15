#include <SDL3/SDL.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_surface.h>
#include <iostream>
#include <mmu.h>
#include <ppu.h>

void PPU::combineObjTile(u8 tileHigh, u8 tileLow, Object *object) {
    u16 line = 0;
    u16 mask = 0;
    line = 0;

    bool flipCond = (object->flags & 0b100000) > 0;

    for (auto j = 0; j < 8; ++j) {
        mask = 0b1 << j;
        line += (((u16)tileHigh & mask) << (j + 1)) + (((u16)tileLow & mask) << j);
    }

    if (flipCond) {
        u16 tempLine = line;
        line = 0;
        for (auto j = 7; j >= 0; --j) {
            line = (line << 2) | (tempLine & 0b11);
            tempLine >>= 2;
        }
    }

    // std::cout << std::setw(4) << std::setfill('0') << std::hex << (int)line <<
    // '\n';
    u8 objQueueSize = objQueue.size();
    u8 xCoord = 0;
    u8 palette = 0;
    u8 bgPriority = 0;
    u8 objIndex = 0;

    for (auto i = 0; i < 8; ++i) {
        u16 mask = 0b11 << 14;
        mask >>= (i * 2);
        u16 color = line & mask;
        color >>= (14 - (i * 2));
        xCoord = object->xPos;
        palette = (object->flags & 0b10000) >> 4;
        bgPriority = (object->flags & 0b10000000) >> 7;
        objIndex = object->objIndex;
        Pixel pixel(color, palette, 0, bgPriority, xCoord, objIndex);
        if (i < objQueueSize) {
            Pixel oldPixel = objQueue.front();
            objQueue.pop();
            if (oldPixel.color == 0 || oldPixel.xCoord > pixel.xCoord)
                objQueue.push(pixel);
            else {
                if (oldPixel.xCoord == pixel.xCoord && pixel.xCoord < oldPixel.xCoord) {
                    objQueue.push(pixel);
                } else {
                    objQueue.push(oldPixel);
                }
            }
        } else
            objQueue.push(pixel);
    }
}

void PPU::combineBGTile(u8 tileHigh, u8 tileLow) {
    u16 line = 0;
    u16 mask = 0;
    line = 0;

    for (auto j = 0; j < 8; ++j) {
        mask = 0b1 << j;
        line += (((u16)tileHigh & mask) << (j + 1)) + (((u16)tileLow & mask) << j);
    }

    for (auto i = 0; i < 8; ++i) {
        u16 mask = 0b11 << 14;
        mask >>= (i * 2);
        u16 color = line & mask;
        color >>= (14 - (i * 2));
        Pixel pixel(color, 0, 0, 0, 0, 0);
        pixel.color = color;
        if (bgQueue.size() < 8)
            bgQueue.push(pixel);
    }
}

u8 PPU::getTileByte(u16 index) { // 2 dots
    return VRAM[index - 0x8000];
}

u16 PPU::bgPixelFetcher() { //  2 dots
    u16 tileMap = 0x9800;
    if ((hw_registers.LCDC & 0b1000) > 0) {
        tileMap = 0x9C00;
    }
    u16 offset = 0xFF & ((u8)(hw_registers.LY + hw_registers.SCY) / 8);
    offset <<= 5;
    offset |= 0x1F & ((u8)(xCoord + hw_registers.SCX) / 8);
    offset &= 0x3FF;
    u16 tileID = VRAM[tileMap + offset - 0x8000];
    u16 tileAddress = (tileID) << 4;
    tileAddress += (((u16)hw_registers.LY + (u16)hw_registers.SCY) % 8) << 1;
    tileAddress += 0b1 << 15;
    bool addressing_method = (hw_registers.LCDC & 0b10000) == 0b10000;
    if (!addressing_method && (tileID & 0x80) == 0) {
        tileAddress += (0b1 << 12);
    }
    return tileAddress;
}

u16 PPU::winPixelFetcher() {
    u16 tileMap = 0x9800;
    if ((hw_registers.LCDC & 0b1000000) > 0) {
        tileMap = 0x9C00;
    }
    u16 offset = 0xFF & (window.yCoord / 8);
    offset <<= 5;
    offset += 0x1F & (window.xCoord / 8);
    u16 tileID = VRAM[tileMap + offset - 0x8000];
    u16 tileAddress = ((u16)tileID) << 4;
    tileAddress += (window.yCoord % 8) << 1;
    tileAddress += 0b1 << 15;
    bool addressing_method = (hw_registers.LCDC & 0b10000) == 0b10000;
    if (!addressing_method && (tileID & 0x80) == 0) {
        tileAddress += (0b1 << 12);
    }
    return tileAddress;
}

void PPU::ppuLoop(u8 ticks) {
    currentLineDots += ticks;
    u8 currentLine = hw_registers.LY; // ly register
    while (currentLine < 144 && finishedLineDots < currentLineDots) {
        if (finishedLineDots >= 456) {
            break;
        }
        if (finishedLineDots < 80 && finishedLineDots < currentLineDots) {
            if (hw_registers.WY <= currentLine) {
                window.WY_cond = true;
            }
            while (finishedLineDots < 80 && finishedLineDots < currentLineDots) {
                u16 address = 0xFEA0 - 2 * (80 - finishedLineDots);
                oamScan(address);
                finishedLineDots += 2;
            }
        }
        if (finishedLineDots >= 76) {
            ppu_state = mode3;
        }
        if (finishedLineDots >= 80 && finishedLineDots < 160 + 80 + mode3_delay &&
                finishedLineDots < currentLineDots) {
            if (finishedLineDots == 80) { // setting up mode3
                while (!bgQueue.empty())
                    bgQueue.pop();
                while (!objQueue.empty())
                    objQueue.pop();
            }
            while (finishedLineDots < 86 &&
                    finishedLineDots <
                    currentLineDots) { // do nothing to simulate discarded tile
                finishedLineDots += 2;
            }
            while (finishedLineDots >= 86 &&
                    finishedLineDots < 160 + 80 + mode3_delay &&
                    finishedLineDots < currentLineDots) {
                if (!fifoFlags.awaitingPush) {
                    fifoFlags.tileAddress = bgPixelFetcher(); // 2 dots
                    fifoFlags.highByte = getTileByte(fifoFlags.tileAddress + 1); // 2 dots
                    fifoFlags.lowByte = getTileByte(fifoFlags.tileAddress); // 2 dots
                    fifoFlags.awaitingPush = true;
                }
                if (!window.WX_cond && bgQueue.empty() && fifoFlags.awaitingPush) {
                    // push new tile row
                    combineBGTile(fifoFlags.highByte, fifoFlags.lowByte); // 2 dots
                    newTile = true;
                    fifoFlags.awaitingPush = false;
                    fifoFlags.fetchLowByte = false;
                    fifoFlags.fetchHighByte = false;
                    fifoFlags.fetchTileID = false;
                }
                for (auto i = 0; i < objFetchIdx; ++i) {
                    if (xCoord == objArr[i].xPos) {
                        mode3_delay += 6;
                        if (newTile) {
                            newTile = false;
                            if (xCoord == 0) {
                                mode3_delay += 5;
                            } else {
                                s8 objPenalty = bgQueue.size() - 1;
                                objPenalty -= 2;
                                objPenalty = (objPenalty < 0) ? 0 : objPenalty;
                                mode3_delay += objPenalty;
                            }
                        }
                        fifoFlags.objTileAddress = 0x8000;
                        fifoFlags.objTileAddress += ((u16)objArr[i].tileIndex)
                            << 4; // assumes tile is not flipped
                        bool flipCond = (objArr[i].flags & 0b1000000) > 0;
                        if (flipCond) {
                            fifoFlags.objTileAddress +=
                                (~(((currentLine - (objArr[i].yPos - 16)) % 8) << 1)) &
                                0b1110;
                        } else
                            fifoFlags.objTileAddress +=
                                ((currentLine - (objArr[i].yPos - 16)) % 8) << 1;
                        if ((hw_registers.LCDC & 0b100) > 0) { // 8x16 tiles
                            if ((objArr[i].yPos - currentLine) > 8) {
                                if (!flipCond) {
                                    fifoFlags.objTileAddress &= ~((u16)0b10000);
                                } else
                                    fifoFlags.objTileAddress |= ((u16)0b10000);
                            } else {
                                if (flipCond) {
                                    fifoFlags.objTileAddress &= ~((u16)0b10000);
                                } else
                                    fifoFlags.objTileAddress |= ((u16)0b10000);
                            }
                        }
                        fifoFlags.objHighByte = getTileByte(fifoFlags.objTileAddress + 1);
                        fifoFlags.objLowByte = getTileByte(fifoFlags.objTileAddress);
                        combineObjTile(fifoFlags.objHighByte, fifoFlags.objLowByte,
                                &objArr[i]);
                    }
                }
                if (firstTile) {
                    for (auto i = 0; i < (hw_registers.SCX % 8); ++i) {
                        bgQueue.pop();
                        // xCoord += 1;
                        mode3_delay += 1;
                    }
                }
                if (xCoord < 168 && ((hw_registers.LCDC & 0b100000) > 0) &&
                        window.WY_cond &&
                        (xCoord - 1 == hw_registers.WX || window.WX_cond)) { // window time
                    if (bgQueue.empty() || window.WX_cond == false) {
                        while (!bgQueue.empty())
                            bgQueue.pop();
                        fifoFlags.tileAddress = winPixelFetcher();
                        window.xCoord += 8;
                        fifoFlags.highByte = getTileByte(fifoFlags.tileAddress + 1);
                        fifoFlags.lowByte = getTileByte(fifoFlags.tileAddress);
                        combineBGTile(fifoFlags.highByte, fifoFlags.lowByte);
                        fifoFlags.awaitingPush = true;
                    }
                    if (!window.WX_cond) {
                        mode3_delay += 6;
                    }
                    window.WX_cond = true;
                }
                if (xCoord > 7 && xCoord < 168) {
                    setPixel(xCoord - 8, currentLine, pixelPicker());
                }
                if (xCoord < 168) {
                    xCoord += 1;
                }
                finishedLineDots += 1;
                if (firstTile)
                    firstTile = false;
                if (!objQueue.empty())
                    objQueue.pop();
                if (!bgQueue.empty())
                    bgQueue.pop();
            }
        }
        if (finishedLineDots >= 156 + 80 + mode3_delay) {
            ppu_state = mode0;
        }
        if (finishedLineDots >= 160 + 80 + mode3_delay && finishedLineDots < 456 &&
                finishedLineDots <= currentLineDots) { // hblank
            firstTile = true;
            while (finishedLineDots < currentLineDots) {
                finishedLineDots += 2;
            }
        }
    }
    if (currentLineDots >= 456) {
        window.WY_cond = false;
        if (window.WX_cond) {
            window.yCoord += 1;
        }
        window.WX_cond = false;
        if (currentLine == 153) {
            currentLine = 0;
            ppu_state = mode2;
        } else
            currentLine += 1;
        hw_registers.LY = currentLine;
        if (currentLine == hw_registers.LYC) { // ly = lyc
            hw_registers.STAT |= 0b100;
        } else
            hw_registers.STAT &= 0b11111011;
        window.xCoord = 0;
        xCoord = 0;
        mode3_delay = 14;
        objArr = {};
        objFetchIdx = 0;
        currentLineDots -= 456;
        finishedLineDots = 0;     // idk tbh?
        if (currentLine == 144) { // vblank
            ppu_state = mode1;
            window.yCoord = 0;
        }
        if (ppu_state != mode1) {
            ppu_state = mode2;
        }
    }
    // std::cout << (int)finishedLineDots << " " << (int)currentLineDots << " " <<
    // (int)ticks << " " << (int)mem.hw_read(LY) << "\n"; std::cout <<
    // (int)currentLineDots << " " << (int)mem.hw_read(LY) << " " << ppu_state <<
    // '\n';
    hw_registers.STAT = (hw_registers.STAT & 0b11111100) | ppu_state;
    // assert(finishedLineDots == currentLineDots);
}

u8 PPU::pixelPicker() {
    if (objQueue.empty() ||
            (objQueue.front().bgPriority == 1 && bgQueue.front().color != 0) ||
            (hw_registers.LCDC & 0b10) == 0 || objQueue.front().color == 0) {
        return (hw_registers.LCDC & 0b1) * ((hw_registers.BGP >> (2 * bgQueue.front().color)) & 0b11);
    } else {
        // there has got to be a better way to do struct member arithmetic
        return (*((&hw_registers.OBP0) + objQueue.front().palette) >>
                (2 * objQueue.front().color)) & 0b11;
    }
}

void PPU::oamScan(u16 address) {    // 2 dots
    u8 currentLine = hw_registers.LY; // ly register
    address -= 0xFE00;
    u8 objY_pos = oam_mem[address];
    Object obj(objY_pos, oam_mem[address + 1], oam_mem[address + 2],
            oam_mem[address + 3], address);
    if ((hw_registers.LCDC & 0b100) > 0) { // 8x16 tiles
        if ((objY_pos - currentLine) > 0 && (objY_pos - currentLine) < 17) {
            if (objFetchIdx < 10) {
                objArr[objFetchIdx] = obj;
                objFetchIdx += 1;
            }
        }
    } else { // 8x8 tiles
        if (((objY_pos - 8) - currentLine) > 0 &&
                ((objY_pos - 8) - currentLine) < 9) {
            if (objFetchIdx < 10) {
                // std::cout << std::hex << (int)address << " " << std::dec <<
                // (int)objFetchIdx << " " << (int)objY_pos << " " << (int)currentLine
                // << "\n";
                objArr[objFetchIdx] = obj;
                objFetchIdx += 1;
            }
        }
    }
}

void PPU::setPixel(u8 w, u8 h, u8 pixel) {
    constexpr static std::array<u16, 4> colors = {0xFFFF, 0xAD6B, 0x5295, 0x0001};
    u16 *pixelAddress = std::bit_cast<u16 *>(surface->pixels);
    pixelAddress += surface->w * h + w;
    *pixelAddress = colors[pixel];
}

void PPU::setSurface(SDL_Texture *texture) {
    SDL_LockTextureToSurface(texture, nullptr, &surface);
}
