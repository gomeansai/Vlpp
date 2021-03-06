/***********************************************************************
Vczh Library++ 3.0
Developer: Zihan Chen(vczh)
Regex::Regular Expression

Classes:
	RegexString						：字符串匹配结果
	RegexMatch						：匹配结果
	Regex							：正则表达式引擎
	RegexToken						：词法记号
	RegexTokens						：词法记号表
	RegexLexer						：词法分析器
***********************************************************************/

#ifndef VCZH_REGEX_REGEX
#define VCZH_REGEX_REGEX

#include "../String.h"
#include "../Pointer.h"
#include "../Collections/List.h"
#include "../Collections/Dictionary.h"

namespace vl
{
	namespace regex_internal
	{
		class PureResult;
		class PureInterpretor;
		class RichResult;
		class RichInterpretor;
	}

	namespace regex
	{

/***********************************************************************
正则表达式引擎数据结构
***********************************************************************/

		/// <summary>A type representing a fragment of the input string.</summary>
		class RegexString : public Object
		{
		protected:
			WString										value;
			vint										start;
			vint										length;

		public:
			RegexString(vint _start=0);
			RegexString(const WString& _string, vint _start, vint _length);

			/// <summary>The position of the input string.</summary>
			/// <returns>The position.</returns>
			vint										Start()const;
			/// <summary>The size of the fragment in characters.</summary>
			/// <returns>The size.</returns>
			vint										Length()const;
			/// <summary>Get the fragment.</summary>
			/// <returns>The fragment.</returns>
			const WString&								Value()const;
			bool										operator==(const RegexString& string)const;
		};

		/// <summary>A type representing a match of the input string.</summary>
		class RegexMatch : public Object, private NotCopyable
		{
			friend class Regex;
		public:
			typedef Ptr<RegexMatch>										Ref;
			typedef collections::List<Ref>								List;
			typedef collections::List<RegexString>						CaptureList;
			typedef collections::Group<WString, RegexString>			CaptureGroup;
		protected:
			collections::List<RegexString>				captures;
			collections::Group<WString, RegexString>	groups;
			bool										success;
			RegexString									result;

			RegexMatch(const WString& _string, regex_internal::PureResult* _result);
			RegexMatch(const WString& _string, regex_internal::RichResult* _result, regex_internal::RichInterpretor* _rich);
			RegexMatch(const RegexString& _result);
		public:
			
			/// <summary>Test if this match is a success match or a failed match. A failed match will only appear when calling [M:vl.regex.Regex.Split] or [M:vl.regex.Regex.Cut]. In other cases, failed matches are either not included in the result, or become null pointers.</summary>
			/// <returns>Returns true if this match is a success match.</returns>
			bool										Success()const;
			/// <summary>Get the whole fragment that matches.</summary>
			/// <returns>The whole fragment.</returns>
			const RegexString&							Result()const;
			/// <summary>Get all fragments that are captured.</summary>
			/// <returns>All fragments that are captured.</returns>
			const CaptureList&							Captures()const;
			/// <summary>Get all fragments that are captured by named groups.</summary>
			/// <returns>All fragments that are captured.</returns>
			const CaptureGroup&							Groups()const;
		};

/***********************************************************************
正则表达式引擎
***********************************************************************/

		/// <summary><![CDATA[
		/// Regular Expression. Here is a brief description of the regular expression grammar:
		///	1) Charset:
		///		a, [a-z], [^a-z]
		/// 2) Functional characters:
		///		^: the beginning of the input (DFA incompatible)
		///		$: the end of the input (DFA incompatible)
		///		regex1|regex2: match either regex1 or regex2
		///	3) Escaping (both \ and / mean the next character is escaped)
		///		Escaped characters:
		///			\r: the CR character
		///			\n: the LF character
		///			\t: the tab character
		///			\s: spacing characters (including space, \r, \n, \t)
		///			\S: non-spacing characters
		///			\d: [0-9]
		///			\D: [^0-9]
		///			\l: [a-zA-Z]
		///			\L: [^a-zA-Z]
		///			\w: [a-zA-Z0-9_]
		///			\W: [^a-zA-Z0-9_]
		///			\.: any character (this is the main different from other regex, which treat "." as any characters and "\." as the dot character)
		///			\\, \/, \(, \), \+, \*, \?, \{, \}, \[, \], \<, \>, \^, \$, \!, \=: represents itself
		///		Escaped characters in charset defined in a square bracket:
		///			\r: the CR character
		///			\n: the LF character
		///			\t: the tab character
		///			\-, \[, \], \\, \/, \^, \$: represents itself
		///	4) Loops:
		///		regex{3}: repeats 3 times
		///		regex{3,}: repeats 3 or more times
		///		regex{1,3}: repeats 1 to 3 times
		///		regex?: repeats 0 or 1 times
		///		regex*: repeats 0 or more times
		///		regex+: repeats 1 or more times
		///		if you add a "?" right after a loop, it means repeating as less as possible (DFA incompatible)
		///	5) Capturing: (DFA incompatible)
		///		(regex): No capturing, just change the operators' association
		///		(?regex): Capture matched fragment
		///		(?<name>regex): Capture matched fragment in a named group called "name"
		///		(<$i>): Match the i-th captured fragment, begins from 0
		///		(<$name;i>): Match the i-th captured fragment in the named group called "name", begins from 0
		///		(<$name>): Match any captured fragment in the named group called "name"
		///	6) MISC
		///		(=regex): The prefix of the following text should match the regex, but it is not counted in the whole match (DFA incompatible)
		///		(!regex): Any prefix of the following text should not match the regex, and it is not counted in the whole match (DFA incompatible)
		///		(<#name>regex): Name the regex "name", and it applies here
		///		(<&name>): Copy the named regex "name" here and apply
		/// ]]></summary>
		class Regex : public Object, private NotCopyable
		{
		protected:
			regex_internal::PureInterpretor*			pure;
			regex_internal::RichInterpretor*			rich;

			void										Process(const WString& text, bool keepEmpty, bool keepSuccess, bool keepFail, RegexMatch::List& matches)const;
		public:
			/// <summary>Create a regular expression.</summary>
			/// <param name="code">The regular expression in a string.</param>
			/// <param name="preferPure">Set to true to tell the Regex to use DFA if possible.</param>
			Regex(const WString& code, bool preferPure=true);
			~Regex();

			/// <summary>Test does the Regex uses DFA to match a string.</summary>
			/// <returns>Returns true if DFA is used.</returns>
			bool										IsPureMatch()const;
			/// <summary>Test does the Regex uses DFA to test a string. Test means ignoring all capturing requirements.</summary>
			/// <returns>Returns true if DFA is used.</returns>
			bool										IsPureTest()const;

			/// <summary>Match a prefix of the text.</summary>
			/// <returns>Returns the match. Returns null if failed.</returns>
			/// <param name="text">The text to match.</param>
			RegexMatch::Ref								MatchHead(const WString& text)const;
			/// <summary>Match a fragment of the text.</summary>
			/// <returns>Returns the match. Returns null if failed.</returns>
			/// <param name="text">The text to match.</param>
			RegexMatch::Ref								Match(const WString& text)const;
			/// <summary>Match a prefix of the text, ignoring all capturing requirements.</summary>
			/// <returns>Returns true if succeeded.</returns>
			/// <param name="text">The text to match.</param>
			bool										TestHead(const WString& text)const;
			/// <summary>Match a fragment of the text, ignoring all capturing requirements.</summary>
			/// <returns>Returns true if succeeded.</returns>
			/// <param name="text">The text to match.</param>
			bool										Test(const WString& text)const;
			/// <summary>Find all matched fragments of the text, returning all matched fragments.</summary>
			/// <param name="text">The text to match.</param>
			/// <param name="matches">All successful matches.</param>
			void										Search(const WString& text, RegexMatch::List& matches)const;
			/// <summary>Split the text by matched fragments, returning all unmatched fragments.</summary>
			/// <param name="text">The text to match.</param>
			/// <param name="keepEmptyMatch">Set to true to keep all empty matches.</param>
			/// <param name="matches">All failed matches.</param>
			void										Split(const WString& text, bool keepEmptyMatch, RegexMatch::List& matches)const;
			/// <summary>Cut the text by matched fragments, returning all matched or unmatched fragments.</summary>
			/// <param name="text">The text to match.</param>
			/// <param name="keepEmptyMatch">Set to true to keep all empty matches.</param>
			/// <param name="matches">All successful and failed matches.</param>
			void										Cut(const WString& text, bool keepEmptyMatch, RegexMatch::List& matches)const;
		};

/***********************************************************************
正则表达式词法分析器
***********************************************************************/

		/// <summary>A token.</summary>
		class RegexToken
		{
		public:
			/// <summary>Position in the input string.</summary>
			vint										start;
			/// <summary>Size of this token in characters.</summary>
			vint										length;
			/// <summary>The token id, begins at 0, represents the regular expression in the list that matches this token. -1 means this token is produced by an error.</summary>
			vint										token;
			/// <summary>The pointer to where this token starts in the input string .</summary>
			const wchar_t*								reading;
			/// <summary>The argument value from [M:vl.regex.RegexLexer.Parse].</summary>
			vint										codeIndex;
			/// <summary>True if this token is complete. False if this token does not end here.</summary>
			bool										completeToken;

			/// <summary>Row number of the first character, begins at 0.</summary>
			vint										rowStart;
			/// <summary>Column number of the first character, begins at 0.</summary>
			vint										columnStart;
			/// <summary>Row number of the last character, begins at 0.</summary>
			vint										rowEnd;
			/// <summary>Column number of the last character, begins at 0.</summary>
			vint										columnEnd;

			bool										operator==(const RegexToken& _token)const;
			bool										operator==(const wchar_t* _token)const;
		};

		/// <summary>Token collection representing the result from the lexical analyzer.</summary>
		class RegexTokens : public Object, public collections::IEnumerable<RegexToken>
		{
			friend class RegexLexer;
		protected:
			regex_internal::PureInterpretor*			pure;
			const collections::Array<vint>&				stateTokens;
			WString										code;
			vint										codeIndex;
			
			RegexTokens(regex_internal::PureInterpretor* _pure, const collections::Array<vint>& _stateTokens, const WString& _code, vint _codeIndex);
		public:
			RegexTokens(const RegexTokens& tokens);

			collections::IEnumerator<RegexToken>*		CreateEnumerator()const;

			/// <summary>Copy all tokens.</summary>
			/// <param name="tokens">Returns all tokens.</param>
			/// <param name="discard">A callback to decide which kind of tokens to discard. The input is [F:vl.regex.RegexToken.token]. Returns true to discard this kind of tokens.</param>
			void										ReadToEnd(collections::List<RegexToken>& tokens, bool(*discard)(vint)=0)const;
		};
		
		/// <summary>Lexical walker.</summary>
		class RegexLexerWalker : public Object
		{
			friend class RegexLexer;
		protected:
			regex_internal::PureInterpretor*			pure;
			const collections::Array<vint>&				stateTokens;
			
			RegexLexerWalker(regex_internal::PureInterpretor* _pure, const collections::Array<vint>& _stateTokens);
		public:
			RegexLexerWalker(const RegexLexerWalker& walker);
			~RegexLexerWalker();
			
			/// <summary>Get the start DFA state number, which represents the correct state before parsing any input.</summary>
			/// <param name="state">The DFA state number.</param>
			vint										GetStartState()const;
			/// <summary>Test if this state can only lead to the end of one kind of token.</summary>
			/// <summary>Returns the token index if this state can only lead to the end of one kind of token. Returns -1 if not.</summary>
			vint										GetRelatedToken(vint state)const;
			/// <summary>Step forward by one character.</summary>
			/// <param name="input">The input character.</param>
			/// <param name="state">The current state. Returns the new current state when this function returns.</param>
			/// <param name="token">Returns the token index at the end of the token.</param>
			/// <param name="finalState">Returns true if it reach the end of the token.</param>
			/// <param name="previousTokenStop">Returns true if the last character is the end of the token.</param>
			void										Walk(wchar_t input, vint& state, vint& token, bool& finalState, bool& previousTokenStop)const;
			/// <summary>Step forward by one character.</summary>
			/// <returns>Returns the new current state.</returns>
			/// <param name="input">The input character.</param>
			/// <param name="state">The current state.</param>
			vint										Walk(wchar_t input, vint state)const;
			/// <summary>Test if the input text is a complete token.</summary>
			/// <param name="input">The input text.</param>
			/// <param name="length">Size of the input text in characters.</param>
			bool										IsClosedToken(const wchar_t* input, vint length)const;
			/// <summary>Test if the input is a complete token.</summary>
			/// <param name="input">The input text.</param>
			bool										IsClosedToken(const WString& input)const;
		};

		/// <summary>Lexical colorizer.</summary>
		class RegexLexerColorizer : public Object
		{
			friend class RegexLexer;
		public:
			typedef void(*TokenProc)(void* argument, vint start, vint length, vint token);

		protected:
			RegexLexerWalker							walker;
			vint										currentState;

			RegexLexerColorizer(const RegexLexerWalker& _walker);
		public:
			RegexLexerColorizer(const RegexLexerColorizer& colorizer);
			~RegexLexerColorizer();

			/// <summary>Reset the colorizer using the DFA state number.</summary>
			/// <param name="state">The DFA state number.</param>
			void										Reset(vint state);
			/// <summary>Step forward by one character.</summary>
			/// <param name="input">The input character.</param>
			void										Pass(wchar_t input);
			/// <summary>Get the start DFA state number, which represents the correct state before colorizing any characters.</summary>
			/// <param name="state">The DFA state number.</param>
			vint										GetStartState()const;
			/// <summary>Get the current DFA state number.</summary>
			/// <param name="state">The DFA state number.</param>
			vint										GetCurrentState()const;
			/// <summary>Colorize a text.</summary>
			/// <param name="input">The text to colorize.</param>
			/// <param name="length">Size of the text in characters.</param>
			/// <param name="tokenProc">Colorizer callback. This callback will be called if any token is found..</param>
			/// <param name="tokenProcArgument">The argument to call the callback.</param>
			void										Colorize(const wchar_t* input, vint length, TokenProc tokenProc, void* tokenProcArgument);
		};

		/// <summary>Lexical analyzer.</summary>
		class RegexLexer : public Object, private NotCopyable
		{
		protected:
			regex_internal::PureInterpretor*			pure;
			collections::Array<vint>					ids;
			collections::Array<vint>					stateTokens;
		public:
			/// <summary>Create a lexical analyzer by a set of regular expression. [F:vl.regex.RegexToken.token] will be the index of the matched regular expression.</summary>
			RegexLexer(const collections::IEnumerable<WString>& tokens);
			~RegexLexer();

			/// <summary>Tokenize a input text.</summary>
			/// <returns>The result.</returns>
			/// <param name="code">The text to tokenize.</param>
			/// <param name="codeIndex">Extra information that will store in [F:vl.regex.RegexToken.codeIndex].</param>
			RegexTokens									Parse(const WString& code, vint codeIndex=-1)const;
			/// <summary>Create a equivalence walker from this lexical analyzer.</summary>
			/// <returns>The walker.</returns>
			RegexLexerWalker							Walk()const;
			/// <summary>Create a equivalence colorizer from this lexical analyzer.</summary>
			/// <returns>The colorizer.</returns>
			RegexLexerColorizer							Colorize()const;
		};
	}
}

#endif