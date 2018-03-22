//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  AppDelegate.h
//  AntescofoConverterApp
//
//  Created by Robert Piéchaud on 11/09/15.
//  Copyright (c) 2017 Antescofo. All rights reserved.
//

#include <Cocoa/Cocoa.h>

@interface ImporterWindow : NSWindow {
    
}

@end

@interface TrackListItem : NSObject {
    NSString* name;
    Boolean   follow;
    Boolean   playback;
    Boolean   enabled;
}

@property (copy) NSString* name;
@property Boolean follow;
@property Boolean playback;
@property Boolean enabled;

- (id)initWithName:(NSString*) aName;

@end

@interface AppDelegate : NSObject <NSTableViewDelegate, NSTableViewDataSource, NSApplicationDelegate>

@property NSMutableArray* trackListData;
@property (weak) IBOutlet NSButton *displayCentsCheckBox;
@property (weak) IBOutlet NSButton *originalPitchesCheckbox;
@property (weak) IBOutlet NSButton *quarterNoteTimeChekbox;
@property (weak) IBOutlet NSButton *compound3_8Checkbox;
@property (weak) IBOutlet NSButton *improveXmlCheckbox;
@property (weak) IBOutlet NSButton *chaseCues;
@property (weak) IBOutlet NSButton *smartGraceNotes;
@property (weak) IBOutlet NSButton *appoggiaturas;
@property (weak) IBOutlet NSTextField *playbackTrackLabel;
@property (weak) IBOutlet NSTextField *versionText;
@property (weak) IBOutlet NSView *rootView;

@property (weak) IBOutlet ImporterWindow* window;
@property (weak) IBOutlet NSTableView *trackList;
@property (weak) IBOutlet NSTextField* displayScoreInfo;
@property (weak) IBOutlet NSButton* importButton;
@property (weak) IBOutlet NSMenuItem *openMenuItem;
@property (weak) IBOutlet NSMenuItem *saveMenuItem;

- (IBAction)import:(id)sender;
- (IBAction)checkboxEvent:(NSButton *)sender;

@end

