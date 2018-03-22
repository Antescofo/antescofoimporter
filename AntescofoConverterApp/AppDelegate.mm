//
// .-. . . .-. .-. .-. .-. .-. .-. .-.
// |-| |\|  |  |-  `-. |   | | |-  | |
// ` ' ' `  '  `-' `-' `-' `-' '   `-'
//
// .-. .  . .-. .-. .-. .-. .-. .-.
//  |  |\/| |-' | | |(   |  |-  |(
// `-' '  ` '   `-' ' '  '  `-' ' '
//
//  AppDelegate.mm
//  AntescofoConverterApp
//
//  Created by Robert Piéchaud on 11/09/15.
//  Copyright (c) 2017 Antescofo. All rights reserved.
//

#include "AppDelegate.h"
#include "ImporterWrapper.h"
#include <vector>

using namespace antescofo;
using namespace std;

@implementation ImporterWindow

- (void)awakeFromNib {
    [self registerForDraggedTypes:[NSArray arrayWithObjects:NSColorPboardType, nil]];
}

- (BOOL)performDragOperation:(id < NSDraggingInfo >)sender {
    NSArray *draggedFilenames = [[sender draggingPasteboard] propertyListForType:NSFilenamesPboardType];
    if ([[[draggedFilenames objectAtIndex:0] pathExtension] isEqual:@"xml"])
        return YES;
    else
        return NO;
}

- (NSDragOperation)draggingEntered:(id < NSDraggingInfo >)sender {
    return NSDragOperationMove;
}

- (NSDragOperation)draggingUpdated:(id<NSDraggingInfo>)sender {
    return NSDragOperationMove;
}
/*
- (BOOL)performDragOperation:(id<NSDraggingInfo>)sender {
    NSPasteboard *pboard = [sender draggingPasteboard];
    NSArray *filenames = [pboard propertyListForType:NSFilenamesPboardType];
    
    if (1 == filenames.count)
        if ([[NSApp delegate] respondsToSelector:@selector(application:openFile:)])
            return [[NSApp delegate] application:NSApp openFile:[filenames lastObject]];
    
    return NO;
}
 */

@end


@implementation TrackListItem
@synthesize name;
@synthesize follow;
@synthesize playback;
@synthesize enabled;

- (id)initWithName:(NSString*) aName
{
    self = [super init];
    self.name = aName;
    self.follow = TRUE;
    self.playback = FALSE;
    self.enabled = TRUE;
    return self;
}
@end

@interface AppDelegate ()

@property ImporterWrapper* importer;
@property int trackAmount;
@property bool invokedWithArguments;

@end

@implementation AppDelegate

@synthesize importButton = _importButton;
@synthesize trackList = _trackList;
@synthesize trackListData = _trackListData;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    _importer = new ImporterWrapper();
    _importer->setOriginalPitches( true );
    _importer->setImproveXml( true );
    _importer->setChaseCues( false );
    _importer->setSmartGraceNotes( false );
    _importer->setAppoggiaturas( true );
    std::string text = "Antescofo Importer " + self.importer->getVersion();
    NSString* display = [[NSString alloc] initWithUTF8String:text.c_str()];
    [self.versionText setStringValue:display];
    self.trackListData = [[NSMutableArray alloc]init];
    //NSString *xmlType = NSCreateFileContentsPboardType(@"xml");
    //NSArray *dragTypes = [NSArray arrayWithObject:xmlType];
    //[self.window registerForDraggedTypes:dragTypes];
    [self parseArguments];
    [self updateScore];
}

-(void) updateInterface
{
    if ( !_invokedWithArguments )
    {
        [self.importButton setTitle:@"Save..."];
        [self.saveMenuItem setTitle:@"Save as Antescofo Score..."];
    }
    if ( self.importer->getInputPath().size() == 0 )
    {
        [self.importButton setEnabled:FALSE];
    }
    [self.saveMenuItem setHidden:(!_invokedWithArguments || self.importer->getInputPath().size() == 0)];
    
    [self.displayCentsCheckBox setEnabled:self.importer->getInputPath().size()];
    [self.originalPitchesCheckbox setEnabled:!self.importer->inputIsMIDI() && self.importer->getInputPath().size()];
    [self.quarterNoteTimeChekbox setEnabled:self.importer->getInputPath().size()];
    [self.compound3_8Checkbox setEnabled:self.importer->getInputPath().size()];
    [self.improveXmlCheckbox setEnabled:self.importer->getInputPath().size()];
    [self.chaseCues setEnabled:self.importer->getInputPath().size()];
    [self.smartGraceNotes setEnabled:self.importer->getInputPath().size()];
    [self.appoggiaturas setEnabled:self.importer->getInputPath().size()];
    
    [self.displayCentsCheckBox setState:self.importer->pitchesAsMidiCents()];
    [self.originalPitchesCheckbox setState:self.importer->hasOriginalPitches()];
    [self.quarterNoteTimeChekbox setState:self.importer->hasQuarterNoteTempo()];
    [self.compound3_8Checkbox setState:self.importer->is3_8_compound()];
    [self.improveXmlCheckbox setState:self.importer->improveXml()];
    [self.chaseCues setState:self.importer->chaseCues()];
    [self.smartGraceNotes setState:self.importer->smartGraceNotes()];
    [self.appoggiaturas setState:self.importer->appoggiaturas()];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    delete _importer;
}

- (void) parseArguments
{
    NSArray *args = [[NSProcessInfo processInfo] arguments];
    std::vector<std::string> stringArgs;
    _invokedWithArguments = false;
    for ( int i = 1; i < [args count]; ++i)
    {
        NSString* tmp = [args objectAtIndex:i];
        stringArgs.push_back([tmp UTF8String]);
    }
    if ( stringArgs.size() )
    {
        _invokedWithArguments = _importer->parseArguments( stringArgs );
    }
    NSLog(@"invoked with ARGS = %d", _invokedWithArguments);
}

- (void) updateWindowTitle
{
    std::string inputPath = _importer->getInputPath();
    if ( inputPath.length() )
    {
        NSString* path = [[NSString alloc] initWithUTF8String:inputPath.c_str()];
        [_window setTitleWithRepresentedFilename:path];
    }
}

- (void) updateScore
{
    NSTableColumn* column = [self.trackList tableColumnWithIdentifier:@"playback"];
    [column setHidden:!self.importer->inputIsMIDI()];
    [self.playbackTrackLabel setHidden:!self.importer->inputIsMIDI()];
    [self updateTrackList];
    [self updateWindowTitle];
    [self updateScoreInfo];
    [self updateInterface];
}

- (void) updateScoreInfo
{
    string scoreInfo;
    if ( _importer->queryScoreInfo( scoreInfo ) )
    {
        scoreInfo += "Number of tracks: " + std::to_string( _trackAmount );
        NSString* display = [[NSString alloc] initWithUTF8String:scoreInfo.c_str()];
        [_displayScoreInfo setStringValue:display];
    }
    else
        [_displayScoreInfo setStringValue:@""];
}

- (void) updateTrackList
{
    [self.trackListData removeAllObjects];
    vector<string> tracks;
    _importer->queryTracklist( tracks );
    if ( tracks.size() )
    {
        _trackAmount = (int) tracks.size();
        for ( int i = 0; i < tracks.size(); ++i )
        {
            [self.trackListData addObject:[[TrackListItem alloc] initWithName:[[NSString alloc] initWithUTF8String:tracks[i].c_str()]]];
            if ( tracks[i].find( "display only") != string::npos )
            {
                ((TrackListItem*)[self.trackListData objectAtIndex:i]).playback = FALSE;
                ((TrackListItem*)[self.trackListData objectAtIndex:i]).follow = FALSE;
                ((TrackListItem*)[self.trackListData objectAtIndex:i]).enabled = FALSE;
            }
        }
    }
    [self.trackList reloadData];
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)pTableViewObj
{
    if ( self.trackListData != nil )
        return [self.trackListData count];
    return 0;
}

- (id)tableView:(NSTableView *)tableView objectValueForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    id item = nil;
    if ( self.trackListData != nil )
    {
        NSString* key = [tableColumn identifier];
        if ( [key compare:@"name"] == NSOrderedSame )
        {
            //NSString* name = [NSString stringWithString:[[self.trackListData objectAtIndex:row] name]];
            NSString* name = [[self.trackListData objectAtIndex:row] name];
            item = name;;
        }
        else if ( [key compare:@"follow"] == NSOrderedSame && ((TrackListItem*)[self.trackListData objectAtIndex:row]).enabled )
        {
            item = [NSNumber numberWithBool:[[self.trackListData objectAtIndex:row] follow]];
        }
        else if ( [key compare:@"playback"] == NSOrderedSame && ((TrackListItem*)[self.trackListData objectAtIndex:row]).enabled )
        {
            item = [NSNumber numberWithBool:[[self.trackListData objectAtIndex:row] playback]];
        }
    }
    return (id) item;
}

- (NSCell *)tableView:(NSTableView *)tableView dataCellForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row
{
    NSCell* item = [tableColumn dataCell];
    NSString* key = [tableColumn identifier];
    if ( [key isEqualToString:@"follow" ])
    {
        NSButtonCell* checkbox = (NSButtonCell*) item;
        [checkbox setState:[[self.trackListData objectAtIndex:row] follow]];
    }
    else if ( [key isEqualToString:@"playback" ])
    {
        NSButtonCell* checkbox = (NSButtonCell*) item;
        [checkbox setState:[[self.trackListData objectAtIndex:row] playback]];
    }
    return item;
}

- (void)tableView:(NSTableView *)tableView setObjectValue:(id)object forTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)rowIndex
{
    NSString* key = [tableColumn identifier];
    if ( [key isEqualToString:@"follow"] && ((TrackListItem*)[self.trackListData objectAtIndex:rowIndex]).enabled )
    {
        BOOL value = [(NSNumber*)object intValue];
        ((TrackListItem*)[self.trackListData objectAtIndex:rowIndex]).follow = value;
        if ( value )
            ((TrackListItem*)[self.trackListData objectAtIndex:rowIndex]).playback = FALSE;
        [tableView reloadData];
    }
    if ( [key isEqualToString:@"playback"] && ((TrackListItem*)[self.trackListData objectAtIndex:rowIndex]).enabled )
    {
        BOOL value = [(NSNumber*)object intValue];
        ((TrackListItem*)[self.trackListData objectAtIndex:rowIndex]).playback = value;
        if ( value )
            ((TrackListItem*)[self.trackListData objectAtIndex:rowIndex]).follow = FALSE;
        [tableView reloadData];
    }
}

- (IBAction)checkboxEvent:(NSButton *)sender
{
    if ( [[sender identifier]  isEqual: @"cents"] )
    {
        [self.originalPitchesCheckbox setEnabled:([sender state] == 0 && !self.importer->inputIsMIDI() )];
    }
    else if ( [[sender identifier]  isEqual: @"quarter"] )
    {
        [self.compound3_8Checkbox setEnabled:([sender state] == 0)];
    }
}

- (IBAction)open:(id)sender
{
    NSArray *fileTypes = [NSArray arrayWithObjects:@"mid",@"midi",@"xml",@"musicxml", @"mxl",nil];
    NSOpenPanel * openDialog = [NSOpenPanel openPanel];
    [openDialog setAllowsMultipleSelection:NO];
    [openDialog setCanChooseDirectories:NO];
    [openDialog setCanChooseFiles:YES];
    [openDialog setFloatingPanel:YES];
    [openDialog setAllowedFileTypes:fileTypes];
    NSInteger result = [openDialog runModal];
    if( result == NSOKButton )
    {
        NSString* tmp = [[[openDialog URLs] objectAtIndex:0] path];
        self.importer->setInputPath( [tmp UTF8String] );
        [self.importButton setEnabled:TRUE];
        [self.saveMenuItem setHidden:FALSE];
        [self updateScore];
    }
}

- (IBAction)import:(id)sender
{
    string output;
    vector<int> trackSelection;
    if ( !self.invokedWithArguments )
    {
        NSSavePanel* saveDialog = [NSSavePanel savePanel];
        [saveDialog setShowsTagField:FALSE];
        //[saveDialog setAllowedFileTypes:[NSArray arrayWithObjects:@"asco.txt",nil]];
        [saveDialog setAllowsOtherFileTypes:FALSE];
        string outputFileName = self.importer->getInputPath();
        outputFileName = outputFileName.substr( 0, outputFileName.find_last_of( "." ) ) + ".asco.txt";
        outputFileName = outputFileName.substr( outputFileName.find_last_of( "/" ) + 1 );
        [saveDialog setNameFieldStringValue:[NSString stringWithUTF8String:outputFileName.c_str()]];
        [saveDialog setCanCreateDirectories:TRUE];
        NSInteger result = [saveDialog runModal];
        if( result == NSOKButton )
        {
            NSString* tmp = [[saveDialog URL] path];
            output = [tmp UTF8String];
            if ( output.substr( output.length() - 9 ) != ".asco.txt" )
            {
                output += ".asco.txt";
            }
            self.importer->setOutputFile( output );
        }
    }
    for ( int i = 0; i < [self.trackListData count]; ++i )
    {
        if ( [[self.trackListData objectAtIndex:i] follow] )
        {
            trackSelection.push_back( i + 1 );
        }
        else if ( [[self.trackListData objectAtIndex:i] playback] )
        {
            trackSelection.push_back( -(i + 1) );
        }
    }
    self.importer->set3_8_compound( [self.compound3_8Checkbox state] );
    self.importer->setOriginalPitches( [self.originalPitchesCheckbox state] );
    self.importer->setQuarterNoteTempo( [self.quarterNoteTimeChekbox state] );
    self.importer->setPitchesAsMidiCents( [self.displayCentsCheckBox state] );
    self.importer->setImproveXml( [self.improveXmlCheckbox state] );
    self.importer->setChaseCues( [self.chaseCues state] );
    self.importer->setSmartGraceNotes( [self.smartGraceNotes state] );
    self.importer->setAppoggiaturas( [self.appoggiaturas state] );
    self.importer->import( trackSelection );
    self.importer->save();
    if ( self.invokedWithArguments )
    {
        [NSApp performSelector:@selector(terminate:) withObject:nil afterDelay:0.0];
    }
}


@end
